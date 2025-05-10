
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

// #define N 256
// #define M 2048

typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;
#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1
// static long int cnt_ops=0;
// static long int cnt_bytes=0;

#define CORE_COUNT 8
#define SIMD_WIDTH_INT 8
#define SIMD_WIDTH_CHAR 32

#define CEIL_DIVISIBLE(x, divi) (((x) + (divi - 1)) & ~(divi - 1) )
#define MIN(a,b) (((a)<(b))?(a):(b))

FILE *f;

/************************************************************************/

// TODO IDEA:
//! Maybe try using the transformed array and as the algorithm does
//! each diag transform the 'done' elements (those above the 'diag' (current antidiag variable))
//! Win -> fast get_antidiag, map_antidiag_to_matrix ?

#include <stdio.h>
#include <immintrin.h>
#include <stdint.h>

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
 * This func shifts all elements of a vector 1 element to the right setting first element to 0
 ? Returns: Shifted vector
 ! Note: Doesn't change given vector
*/
__m256i _mm256_shift_right_1_epi32(__m256i v) {
    // Shift indices: move lanes 0–6 into 1–7, first lane becomes zero
    __m256i indices = _mm256_set_epi32(
        6, 5, 4, 3, 2, 1, 0, 255
    );
    return _mm256_permutevar8x32_epi32(v, indices);
}

/*
 * This func gets a specified (diag) antidiagonal from a matrix
 ? Returns: Int* containing all elements of the antidiagonal
 ! Note: Manual free needed, uses openmp parallelization
*/
int *get_antidiag(const int *matrix, int diag, int diagLen, int cols) {
    int *antidiag = (int *)malloc(diagLen * sizeof(int));
    if (!antidiag) return NULL;
    
    #pragma omp parallel for schedule(static) num_threads(CORE_COUNT)
    for (int row = 0; row < diagLen; ++row) {
        antidiag[row] = matrix[row * cols + diag - row];
    }

    return antidiag;
}

/*
 * This func stores a given to a specific antidiagonal (diag) of a matrix 
 ? Returns: Nothing
 ! Note: Does NOT check bounds user specifies diagLen, uses openmp parallelization
*/
void map_antidiag_to_matrix(int *matrix, const int *antidiag, int diag, int diagLen, int cols) {
    #pragma omp parallel for schedule(static)
    for (int row = 0; row < diagLen; ++row) {
        int col = diag - row;
        matrix[row * cols + col] = antidiag[row];
    }
}

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

void compute_result(int diag, int *similarity_matrix, int *cur_diag, int *prev_diag,
                    int *prev_prev_diag, int vectorNum, const int maxDiagLen, const int N, const int M) 
{
    prev_prev_diag = prev_diag;
    prev_diag = cur_diag;
	cur_diag = get_antidiag(similarity_matrix, diag, maxDiagLen, N); //* Partial result

    __m256i prev_prev_vec, zero_vec, prev_vec, prev_shifted_vec, cur_vec, max_vec;

    #pragma omp parallel for schedule(static) num_threads(CORE_COUNT) private(prev_prev_vec, zero_vec, prev_vec, prev_shifted_vec, cur_vec, max_vec)
    for (int i = 0; i < vectorNum; i++) {
        // Initialization //* Set values from diagonal arrays 
        prev_prev_vec = _mm256_loadu_si256((__m256i*)&prev_prev_diag[i << 3]);
        prev_vec = _mm256_loadu_si256((__m256i*)&prev_diag[i << 3]);
        cur_vec = _mm256_loadu_si256((__m256i*)&cur_diag[i << 3]);
        zero_vec = _mm256_setzero_si256();

        // Setup vectors //* Perform operations -1 for W and N and += Match for NW
        prev_vec = _mm256_add_epi32(prev_vec, _mm256_set1_epi32(GAP_i));
        prev_shifted_vec = _mm256_shift_right_1_epi32(prev_vec);
        prev_prev_vec = _mm256_add_epi32(prev_prev_vec, cur_vec); //* cur is mm vector

        // Find max
        max_vec = _mm256_max_epi32(prev_vec, prev_shifted_vec);
        max_vec = _mm256_max_epi32(max_vec, prev_prev_vec);
        max_vec = _mm256_max_epi32(max_vec, zero_vec);

        // Write to matrix //* Write the calculated values back to the similarity_matrix 
        _mm256_storeu_si256((__m256i*)&cur_diag[i << 3], max_vec);

        // TODO Maybe save in temp array here first and call this outside of the loop
        map_antidiag_to_matrix(similarity_matrix, cur_diag, diag, maxDiagLen, N);
    }
}

void printSimilarityMatrix(int *similarity_matrix, const char *query, const char *database, const int N, const int M) {
	printf("################# Similarity Matrix #################\n");

	//Print query string
	printf("  ");
	for (int i = 0; i < M; i++) {
		printf("%3c", database[i]);
	}
	printf("\n");

	//Print matrix and database
	for (int j = 0; j < N; j++) {
		if (j >= 0)
			printf("%c ", query[j]);

		for (int i = 0; i < M ; i++) {
			int index = j * M + i;
			printf("%3d", similarity_matrix[index]);
		}

		printf("\n");
	}

	printf("#####################################################\n");
}

void compute_matrices(
    const char *string1, const char *string2,
    int *max_index, int *similarity_matrix,
    short *direction_matrix, const int N, const int M)
{
    const int maxDiagLen = MIN(N, M);
    int diagNum = N + M - 1;
    int vectorNumR = (maxDiagLen + SIMD_WIDTH_INT - 1) / SIMD_WIDTH_INT;
    
    //* Allocate antidiagonal arrays
    int *prev_diag = (int *) malloc(maxDiagLen * sizeof(int));
    if (!prev_diag) goto free_prv;

    int *prev_prev_diag = (int *) malloc(maxDiagLen * sizeof(int));
    if (!prev_prev_diag) goto free_prv_prv;
    
    int *cur_diag = (int *) malloc(maxDiagLen * sizeof(int));
    if (!cur_diag) goto free_cur;

    //* Enable thread nesting
    omp_set_nested(1); //! Do not move inside #pragmas
    
    compute_missmatch(similarity_matrix, string1, string2, N, M);
    printSimilarityMatrix(similarity_matrix, string1, string2, N, M);
    
    for (int diag = 0; diag < diagNum; ++diag)
        compute_result(diag, similarity_matrix, cur_diag, prev_diag, prev_prev_diag, vectorNumR, maxDiagLen, N, M);
    
    //* Free ladder
    free_cur:
        free(cur_diag);
    free_prv_prv:
        free(prev_prev_diag);
    free_prv:
        free(prev_diag);
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

void fillLetter(char* string, int dimension, const char letter) {
	for (int i = 0; i < dimension; i++) string[i] = letter;
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
    
    f = fopen("VECTOR_OUT.txt", "w"); // Or "/tmp/my_fifo" for a named pipe
    if (!f) {
        perror("fopen");
        return 1;
    }
    
	if (argc != 3) {
        printf("%s <Query Size N> <DataBase Size M>\n", argv[0]);
		return EXIT_FAILURE;
	}
    
    printf("Starting Local Alignment Code \n");
	fflush(stdout);
    
	/* Typically, M >> N */
	int N = atoi(argv[1]); 
    int M = atoi(argv[2]);
    
    int num_pad_cols = CEIL_DIVISIBLE(N, 32);
    const int maxDiagLen = MIN(num_pad_cols, M);
    int num_pad_rows = M + (maxDiagLen << 1);
    int similarityLen = num_pad_cols * num_pad_rows;

    char *query = (char*) malloc(sizeof(char) * num_pad_cols);              //"TGTTACGGTAGTGTTACGGTAG";
	char *database = (char*) malloc(sizeof(char) * num_pad_rows);    //"GGTTGACTAGGTTGACTAGGTTGACTA";
	int *similarity_matrix = (int*) malloc(sizeof(int) * similarityLen);
	short *direction_matrix = (short*) malloc(sizeof(short) * num_pad_cols * M); // TODO FIX 
	int *max_index = (int *) malloc(sizeof(int));

	/* Create the two input strings by calling a random number generator */
	fillRandom(query, N);
    fillLetter(&query[N], num_pad_cols - N, 'X');
    fillLetter(database, maxDiagLen, 'X');
	fillRandom(&database[maxDiagLen], M);
    fillLetter(&database[maxDiagLen + M], maxDiagLen, 'X');

	memset(similarity_matrix, 0, sizeof(int) * similarityLen);
	memset(direction_matrix, 0, sizeof(short) * N * M); // TODO FIX 

    int queryLen = num_pad_cols;    // TODO Remove later readability for now 
    int databaseLen = num_pad_rows; // TODO Remove later readability for now
    
    double start, end;
    start = omp_get_wtime();
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, queryLen, databaseLen);
	end = omp_get_wtime();

#ifdef ENABLE_TEST_OUTPUT
    printSimilarityMatrix(similarity_matrix, query, database, queryLen, databaseLen);
#endif

    printf("max index is in position (%d, %d) \n", max_index[0]/(num_pad_cols), max_index[0]%(num_pad_cols));
	printf("execution time of LSAL SW algorithm is %f sec \n", end - start);
	
	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);
	free(query);
	free(database);

    fclose(f);
    // printf("cnt_ops=%d, cnt_bytes=%d\n", cnt_ops, cnt_bytes);
	return EXIT_SUCCESS;
}
