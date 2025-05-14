/* Optimizations:
   - Reuse of already fetched elements used previously
*/

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
#include <immintrin.h>


typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;
#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1
#define CEIL_TO_DIVISIBLE(target, val)  (((val) + (target-1)) & ~(target-1))

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
            char vals[32];
            _mm256_storeu_si256((__m256i*)vals, vec);
            for (int i = 0; i < 32; ++i) {
                if (unsigned_input)
                    fprintf(stream, "%u ", (char)vals[i]);
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

/**********************************************************************************************
 * LSAL algorithm
 * Inputs:
 *          string1 is the query[N]
 *          string2 is the database[M]
 *          input sizes N, M
 * Outputs:
 *           max_index is the location of the highest similiarity score 
 *           similarity and direction matrices. Note that these two matrices are initialized with zeros.
 **********************************************************************************************/
#pragma GCC optimize("O3", "unroll-loops", "prefetch-loop-arrays")
#pragma GCC target("arch=znver3")
void compute_matrices(
	const char *string1, const char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix, const int N, const int M, const int smallPadding)
{
    int i, j;
	int vec = 16;
	long int similarity_index = N + 2 + smallPadding;
	long int dir_idx = 0;

    int north_value = 0;
	int max_value = 0;

	short direction = 0;
	*max_index = N + 1;

	__m256i match_val = _mm256_set1_epi8(2);
	__m256i mismatch_val = _mm256_set1_epi8(-1);
	
	#pragma GCC unroll 2048
	for (i = 0; i < M; i++, vec++) {
		j = 0; //! Important
		__builtin_prefetch(string2 + (j << 6), 0, 1);
		__builtin_prefetch(string1 + (j << 6), 0, 1);
		
		int northwest_value = 0;
		int west_value = 0;
		
		__m256i str2_vec = _mm256_set1_epi8(string2[i]);
		__m256i str1_vec = _mm256_loadu_si256((__m256i*)&string1[j]);
		__m256i cmps_vec = _mm256_cmpeq_epi8(str1_vec, str2_vec);
		__m256i result = _mm256_blendv_epi8(mismatch_val, match_val, cmps_vec);
				
		for (j = 0; j < N; j++, dir_idx++, similarity_index++, vec++) {

			if (!(vec%16)) {
				str1_vec = _mm256_loadu_si256((__m256i*)&string1[j]);
				cmps_vec = _mm256_cmpeq_epi8(str1_vec, str2_vec);
				result = _mm256_blendv_epi8(mismatch_val, match_val, cmps_vec);
			}

			northwest_value += (signed char)_mm256_extract_epi8(result, 0);
			result = _mm256_srli_si256(result, 1);

			north_value = similarity_matrix[similarity_index - smallPadding - N - 1] + GAP_d;

			max_value = northwest_value;
			direction = NORTH_WEST;

			int cond_north = (north_value > max_value);
			max_value = cond_north ? north_value : max_value;
			direction = cond_north ? NORTH : direction;

			int cond_west = (west_value > max_value);
			max_value = cond_west ? west_value : max_value;
			direction = cond_west ? WEST : direction;

			int cond_zero = (0 >= max_value);
			max_value = cond_zero ? 0 : max_value;
			direction = cond_zero ? CENTER : direction;

			similarity_matrix[similarity_index] = max_value;
			direction_matrix[dir_idx] = direction;
			
			*max_index = max_value > similarity_matrix[*max_index] ? similarity_index : *max_index;
			northwest_value = north_value - GAP_d;
			west_value = max_value + GAP_i;
		}
		similarity_index += smallPadding + 1;
	}
}  // end of function

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
			printf("%3d", (signed char)similarity_matrix[index]);
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
	long int N = atoi(argv[1]); 
    long int M = atoi(argv[2]);

    char *query = (char*) aligned_alloc(32, sizeof(char) * N);//"TGTTACGGTAGTGTTACGGTAG";
	char *database = (char*) aligned_alloc(32, sizeof(char) * M);//"GGTTGACTAGGTTGACTAGGTTGACTA";
	int *similarity_matrix = (int*) malloc(sizeof(int) * ((N + 1) * (M + 1)));
	short *direction_matrix = (short*) malloc(sizeof(short) * N * M);
	int *max_index = (int *) malloc(sizeof(int));

	/* Create the two input strings by calling a random number generator */
	fillRandom(query, N);
	fillRandom(database, M);

	memset(similarity_matrix, 0, sizeof(int) * ((N + 1) * (M + 1)));
	memset(direction_matrix, 0, sizeof(short) * N * M);

    t1 = clock();
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, N, M, CEIL_TO_DIVISIBLE(32, N) - N);
	t2 = clock();

#ifdef ENABLE_TEST_OUTPUT
	printSimilarityMatrix(similarity_matrix, query, database, N, M);
	printDirectionMatrix(direction_matrix, N, M);
#endif

    printf("max index is in position (%ld, %ld) \n", max_index[0]/(N + 1), max_index[0]%(N+1));
	printf("execution time of LSAL SW algorithm is %f sec \n", (double)(t2-t1) / CLOCKS_PER_SEC);
	
	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);
	free(query);
	free(database);

    // printf("cnt_ops=%d, cnt_bytes=%d\n", cnt_ops, cnt_bytes);
	return EXIT_SUCCESS;
}