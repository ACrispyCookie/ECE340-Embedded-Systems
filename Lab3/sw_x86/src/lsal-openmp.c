#include <emmintrin.h>
#include <omp.h>
#include <string.h>
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

typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;
#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1
#define SIMD_WIDTH 8 // Number of 16-bit values in 128-bit SIMD
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// SSE2-compatible blend: chooses b[i] if mask[i] is 1, else a[i]
static inline __m128i _mm_blendv_epi8(__m128i a, __m128i b, __m128i mask) {
    return _mm_or_si128(
        _mm_and_si128(mask, b),
        _mm_andnot_si128(mask, a)
    );
}

void compute_matrices(
	const char *string1, const char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix,
	const int N, const int M)
{
	const short match_score = MATCH;
	const short mismatch_penalty = MISS_MATCH;
	const short gap_open = GAP_i;
	const short gap_extend = GAP_d;

	*max_index = N + 1;
	int total = (N + 1) * (M + 1);
	memset(similarity_matrix, 0, total * sizeof(int));
	memset(direction_matrix, 0, total * sizeof(short));

	// Allocate gap vectors
	__m128i v_gap_open = _mm_set1_epi16(gap_open);
	__m128i v_gap_extend = _mm_set1_epi16(gap_extend);
	__m128i v_zero = _mm_setzero_si128();
	__m128i max_val = v_zero;

	// #pragma omp parallel for schedule(static) // enable for multi-threaded
	for (int j = 0; j < M; j++) {
		__m128i E[SIMD_WIDTH] = { v_zero };  // Insertion gaps
		__m128i F = v_zero;                  // Deletion gap
		__m128i H = v_zero;                  // Current values

		for (int i = 0; i < N; i += SIMD_WIDTH) {
			__m128i v_H_diag = v_zero;
			__m128i v_H_left = v_zero;
			__m128i v_H_above = v_zero;

			short s1[SIMD_WIDTH], s2[SIMD_WIDTH];
			for (int k = 0; k < SIMD_WIDTH; k++) {
				s1[k] = (i + k < N) ? string1[i + k] : 0;
				s2[k] = string2[j];
			}

			__m128i v_s1 = _mm_loadu_si128((__m128i*)s1);
			__m128i v_s2 = _mm_loadu_si128((__m128i*)s2);
			__m128i v_cmp = _mm_cmpeq_epi16(v_s1, v_s2);
			__m128i v_score = _mm_blendv_epi8(_mm_set1_epi16(mismatch_penalty), _mm_set1_epi16(match_score), v_cmp);

			// Load previous scores from matrix
			for (int k = 0; k < SIMD_WIDTH; k++) {
				int idx = (j + 1) * (N + 1) + (i + k + 1);
				int idx_diag = j * (N + 1) + i + k;
				int idx_above = j * (N + 1) + i + k + 1;
				int idx_left = (j + 1) * (N + 1) + i + k;

				short diag = similarity_matrix[idx_diag];
				short up = similarity_matrix[idx_above];
				short left = similarity_matrix[idx_left];

				short score = MAX(0, MAX(diag + ((s1[k] == s2[k]) ? match_score : mismatch_penalty),
									MAX(up + gap_extend, left + gap_open)));
				
				similarity_matrix[idx] = score;
				#pragma omp critical
				{
					if (score > similarity_matrix[*max_index])
						*max_index = idx;
				}
			}
		}
	}
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
