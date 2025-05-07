#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <emmintrin.h>  // SSE2
#include <smmintrin.h>  // SSE4.1 for _mm_blendv_epi8

// #define N 256
// #define M 2048

typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;
#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1
// static long int cnt_ops=0;
// static long int cnt_bytes=0;

#define SIMD_WIDTH 8
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -1

void compute_matrices(
    const char *string1, const char *string2,
    int *max_index, int *similarity_matrix,
    short *direction_matrix, const int N, const int M)
{
    int segLen = (N + SIMD_WIDTH - 1) / SIMD_WIDTH;
    __m128i *VHStore = (__m128i*)_mm_malloc(segLen * sizeof(__m128i), 16);
    __m128i *VHLoad = (__m128i*)_mm_malloc(segLen * sizeof(__m128i), 16);
    __m128i vZero = _mm_setzero_si128();
    __m128i vGap = _mm_set1_epi16(GAP_PENALTY);
    __m128i vMatch = _mm_set1_epi16(MATCH_SCORE);
    __m128i vMismatch = _mm_set1_epi16(MISMATCH_SCORE);

    for (int i = 0; i < segLen; ++i) {
        VHStore[i] = vZero;
        VHLoad[i] = vZero;
    }

    *max_index = N + 1;
    int max_score = 0;

    for (int dbIndex = 0; dbIndex < M; dbIndex++) {
        char dbChar = string2[dbIndex];
        __m128i vF = vZero;

        __m128i* tmp = VHLoad;
        VHLoad = VHStore;
        VHStore = tmp;

        __m128i vH = VHLoad[segLen - 1];
        vH = _mm_slli_si128(vH, 2);  // Shift left by 2 bytes (1 short)

        for (int segIdx = 0; segIdx < segLen; ++segIdx) {
            short s1[SIMD_WIDTH];
            for (int k = 0; k < SIMD_WIDTH; ++k) {
                int idx = k * segLen + segIdx;
                s1[k] = (idx < N) ? string1[idx] : 0;
            }

            short s2[SIMD_WIDTH];
            for (int k = 0; k < SIMD_WIDTH; ++k) s2[k] = dbChar;

            __m128i vS1 = _mm_loadu_si128((__m128i*)s1);
            __m128i vS2 = _mm_loadu_si128((__m128i*)s2);
            __m128i vCmp = _mm_cmpeq_epi16(vS1, vS2);
            __m128i vScore = _mm_blendv_epi8(vMismatch, vMatch, vCmp);

            vH = _mm_adds_epi16(vH, vScore);  // vH = H[i-1][j-1] + match/mismatch

            __m128i vHload = VHLoad[segIdx];
            vH = _mm_max_epi16(vH, vHload + vGap);  // left neighbor - gap
            vH = _mm_max_epi16(vH, vF);             // top neighbor F (gap from above)
            vH = _mm_max_epi16(vH, vZero);          // clamp to 0

            VHStore[segIdx] = vH;

            vF = _mm_subs_epi16(vF, vGap);
            vF = _mm_max_epi16(vF, vHload + vGap);

            // Save into similarity_matrix
            for (int k = 0; k < SIMD_WIDTH; ++k) {
                int row = k * segLen + segIdx;
                int col = dbIndex;
                if (row < N) {
                    int simIdx = (col + 1) * (N + 1) + (row + 1);
                    short val = _mm_extract_epi16(vH, k);
                    similarity_matrix[simIdx] = val;
                    if (val > max_score) {
                        max_score = val;
                        *max_index = simIdx;
                    }
                }
            }

            vH = vHload;
        }

        // Lazy F loop (simplified since no gap open/extend distinction)
        for (int k = 0; k < SIMD_WIDTH; ++k) {
            short carryF = _mm_extract_epi16(vF, k);
            if (carryF <= 0) continue;
            for (int segIdx = 0; segIdx < segLen; ++segIdx) {
                __m128i vHval = VHStore[segIdx];
                short curr = _mm_extract_epi16(vHval, k);
                carryF = carryF + GAP_PENALTY;
                if (carryF > curr) {
                    curr = carryF;
                    vHval = _mm_insert_epi16(vHval, curr, k);
                    VHStore[segIdx] = vHval;
                    int row = k * segLen + segIdx;
                    int col = dbIndex;
                    if (row < N) {
                        int simIdx = (col + 1) * (N + 1) + (row + 1);
                        similarity_matrix[simIdx] = curr;
                        if (curr > max_score) {
                            max_score = curr;
                            *max_index = simIdx;
                        }
                    }
                } else {
                    break;
                }
            }
        }
    }

    _mm_free(VHLoad);
    _mm_free(VHStore);
}

/************************************************************************/

/*
 return a random number in [0, limit].
 */
int rand_lim(int limit) {

	int divisor = RAND_MAX / (limit + 1);
	int retval;

	do {
		retval = rand() / divisor;
	} while (retval > limit);

	return retval;
}

/*
 Fill the string with random values
 */
void fillRandom(char* string, int dimension) {
	//fill the string with random letters..
	static const char possibleLetters[] = "ATCG";

	string[0] = '-';

	int i;
	for (i = 0; i < dimension; i++) {
		int randomNum = rand_lim(3);
		string[i] = possibleLetters[randomNum];
	}

}

void printSimilarityMatrix(int *similarity_matrix, char *query, char *database, const int N, const int M) {
	printf("################# Similarity Matrix #################\n");

	//Print query string
	printf("     ");
	for (int i = 0; i < N; i++) {
		printf("%3c", query[i]);
	}
	printf("\n");

	//Print matrix and database
	printf("  ");
	for (int j = 0; j < M + 1; j++) {
		if (j > 0)
			printf("%c ", database[j - 1]);

		for (int i = 0; i < N + 1; i++) {
			int index = j * (N + 1) + i;
			printf("%3d", similarity_matrix[index]);
		}

		printf("\n");
	}

	printf("#####################################################\n");
}

void printDirectionMatrix(short *direction_matrix, const int N, const int M) {
	printf("################# Direction Matrix #################\n");
	for (int j = 0; j < M; j++) {
		for (int i = 0; i < N; i++) {
			int index = j * N + i;

			char *s;
			switch (direction_matrix[index]) {
				case NORTH_WEST:
					s = "NW";
					break;
				case NORTH:
					s = "N";
					break;
				case WEST:
					s = "W";
					break;
				case CENTER:
				default:
					s = "C";
					break;
			}
			printf("%2s ", s);
		}
		printf("\n");
	}
	printf("####################################################\n");
}

/* ******************************************************************/
int main(int argc, char** argv) {

    clock_t t1, t2;

	if (argc != 3) {
		printf("%s <Query Size N> <DataBase Size M>\n", argv[0]);
		return EXIT_FAILURE;
	}
 
    printf("Starting Local Alignment Code \n");
	fflush(stdout);

	/* Typically, M >> N */
	int N = atoi(argv[1]); 
    int M = atoi(argv[2]);

    char *query = (char*) malloc(sizeof(char) * N);//"TGTTACGGTAGTGTTACGGTAG";
	char *database = (char*) malloc(sizeof(char) * M);//"GGTTGACTAGGTTGACTAGGTTGACTA";
	int *similarity_matrix = (int*) malloc(sizeof(int) * ((N + 1) * (M + 1)));
	short *direction_matrix = (short*) malloc(sizeof(short) * N * M);
	int *max_index = (int *) malloc(sizeof(int));

	/* Create the two input strings by calling a random number generator */
	fillRandom(query, N);
	fillRandom(database, M);

	memset(similarity_matrix, 0, sizeof(int) * ((N + 1) * (M + 1)));
	memset(direction_matrix, 0, sizeof(short) * N * M);

    t1 = clock();
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, N, M);
	t2 = clock();

#ifdef ENABLE_TEST_OUTPUT
	printSimilarityMatrix(similarity_matrix, query, database, N, M);
	printDirectionMatrix(direction_matrix, N, M);
#endif

    printf("max index is in position (%d, %d) \n", max_index[0]/(N + 1), max_index[0]%(N+1));
	printf("execution time of LSAL SW algorithm is %f sec \n", (double)(t2-t1) / CLOCKS_PER_SEC);
	
	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);
	free(query);
	free(database);

    // printf("cnt_ops=%d, cnt_bytes=%d\n", cnt_ops, cnt_bytes);
	return EXIT_SUCCESS;
}
