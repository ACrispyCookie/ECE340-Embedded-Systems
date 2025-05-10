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
#include <omp.h>
#include <immintrin.h>
#include <stdint.h>

#define CORE_COUNT 8
#define SIMD_WIDTH_INT 8
#define SIMD_WIDTH_CHAR 32

typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;
#define GAP_i -1
#define GAP_d -1
#define CEIL_TO_32(x)  (((x) + 31) & ~31)
FILE *f;

void compute_missmatch(int *similarity_matrix, const char *string1, const char *string2, const int N, const int M) {
    #pragma omp parallel num_threads(CORE_COUNT)
    {  
        __m256i str1_vec, str2_vec, cmps_vec, match_val, mismatch_val, result, cnv0_vec, cnv1_vec, cnv2_vec, cnv3_vec;
        __m128i low, high;
        //* Set for to static (same workload amongst threads) 
        #pragma omp for schedule(static) private(str1_vec, str2_vec, cmps_vec, result, cnv0_vec, cnv1_vec, cnv2_vec, cnv3_vec, low, high)
        for (long int j = M - 1; j >= 0; j--) {
            long int similarity_index = (j + 2) * (N + 1) - SIMD_WIDTH_CHAR;
            for (long int idx = N - SIMD_WIDTH_CHAR; idx >= 0; idx -= SIMD_WIDTH_CHAR, similarity_index -= SIMD_WIDTH_CHAR) {
                str1_vec = _mm256_loadu_si256((__m256i*)&string1[idx]);
                // print_m256i_to(f, str1_vec, "str1_vec: ", 8, 0);
                str2_vec = _mm256_set1_epi8(string2[j]);
                // print_m256i_to(f, str2_vec, "str2_vec: ", 8, 0);
                cmps_vec = _mm256_cmpeq_epi8(str1_vec, str2_vec);
                // print_m256i_to(f, cmps_vec, "cmps_vec: ", 8, 0);
                
                // Set values for blendv //* Sets value for packed int of length (epiX -> epi8) 8bits           
                match_val = _mm256_set1_epi8(2);
                // print_m256i_to(f, match_val, "match_val: ", 8, 0);
                mismatch_val = _mm256_set1_epi8(-1);
                // print_m256i_to(f, mismatch_val, "mismatch_val: ", 8, 0);
                
                // Calculation &Alignments //* Calculate vector with missmatch values
                result = _mm256_blendv_epi8(mismatch_val, match_val, cmps_vec);
                // print_m256i_to(f, result, "result: ", 8, 0);

                low = _mm256_castsi256_si128(result);
                high = _mm256_extracti128_si256(result, 1);
                cnv0_vec = _mm256_cvtepi8_epi32(low);
                // print_m256i_to(f, cnv0_vec, "cnv0_vec: ", 32, 0);
                cnv1_vec = _mm256_cvtepi8_epi32(_mm_srli_si128(low, 8));
                // print_m256i_to(f, cnv1_vec, "cnv1_vec: ", 32, 0);
                cnv2_vec = _mm256_cvtepi8_epi32(high);
                // print_m256i_to(f, cnv2_vec, "cnv2_vec: ", 32, 0);
                cnv3_vec = _mm256_cvtepi8_epi32(_mm_srli_si128(high, 8));
                // print_m256i_to(f, cnv3_vec, "cnv3_vec: ", 32, 0);
                
                // fprintf(f, "j * M + idx: %d, j: %d, idx: %d\n", similarity_index, j, idx);
                _mm256_storeu_si256((__m256i*)&similarity_matrix[similarity_index +  0], cnv0_vec);
                _mm256_storeu_si256((__m256i*)&similarity_matrix[similarity_index +  8], cnv1_vec);
                _mm256_storeu_si256((__m256i*)&similarity_matrix[similarity_index + 16], cnv2_vec);
                _mm256_storeu_si256((__m256i*)&similarity_matrix[similarity_index + 24], cnv3_vec);

            }
        }
    }
}

void print_m256i_to(FILE *stream, __m256i vec, const char *prefix, int bit_size, int unsigned_input) {
    #pragma omp critical
    {
        fprintf(stream, "%s: ", prefix);
        if (bit_size == 32) {
            int vals[8];
            _mm256_storeu_si256((__m256i*)vals, vec);
            for (int i = 0; i < 8; ++i) {
                if (unsigned_input)
                    fprintf(stream, "%u ", (unsigned int)vals[i]);
                else
                    fprintf(stream, "%d ", vals[i]);
            }
        } else if (bit_size == 16) {
            short vals[16];
            _mm256_storeu_si256((__m256i*)vals, vec);
            for (int i = 0; i < 16; ++i) {
                if (unsigned_input)
                    fprintf(stream, "%u ", (unsigned short)vals[i]);
                else
                    fprintf(stream, "%d ", vals[i]);
            }
        } else if (bit_size == 8) {
            int8_t vals[32];
            _mm256_storeu_si256((__m256i*)vals, vec);
            for (int i = 0; i < 32; ++i) {
                if (unsigned_input)
                    fprintf(stream, "%u ", (uint8_t)vals[i]);
                else
                    fprintf(stream, "%d ", vals[i]);
            }
        } else {
            fprintf(stream, "[Unsupported bit size: %d]", bit_size);
        }

        fprintf(stream, "\n");
        fflush(stream); // Force print

    }
}

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

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("%s <Query Size N> <DataBase Size M>\n", argv[0]);
		return EXIT_FAILURE;
	}
    printf("Starting Local Alignment Code \n");
	fflush(stdout);
    f = stdout;
	
    /* Typically, M >> N */
	long int N = atoi(argv[1]); 
    long int M = atoi(argv[2]);

    char *query = (char*) malloc(sizeof(char) * N);
	char *database = (char*) malloc(sizeof(char) * M);
	int *similarity_matrix = (int*) malloc(sizeof(int) * ((N + 1) * (M + 1)));
	short *direction_matrix = (short*) malloc(sizeof(short) * N * M);
	int *max_index = (int *) malloc(sizeof(int));
	fillRandom(query, N);
	fillRandom(database, M);

	memset(direction_matrix, 0, sizeof(short) * N * M);

    double start, end1, end2;
    start = omp_get_wtime();
    compute_missmatch(similarity_matrix, query, database, N, M);
	end1 = omp_get_wtime();
    //compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, N, M);
	end2 = omp_get_wtime();

#ifdef ENABLE_TEST_OUTPUT
	printSimilarityMatrix(similarity_matrix, query, database, N, M);
	printDirectionMatrix(direction_matrix, N, M);
#endif

    printf("max index is in position (%ld, %ld) \n", max_index[0]/(N + 1), max_index[0]%(N+1));
	printf("execution time of compute missmatch is %f sec \n", end1 - start);
	printf("execution time of compute matrices is %f sec \n", end2 - end1);
	printf("execution time of LSAL SW algorithm is %f sec \n", end2 - start);

	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);
	free(query);
	free(database);

    // printf("cnt_ops=%d, cnt_bytes=%d\n", cnt_ops, cnt_bytes);
	return EXIT_SUCCESS;
}