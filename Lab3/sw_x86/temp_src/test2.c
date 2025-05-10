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

void compute_result(int diag, int *similarity_matrix, int *cur_diag, int *prev_diag,
                    int *prev_prev_diag, int vectorNum, const int maxDiagLen, const int N) 
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
    
    for (int diag = 0; diag < diagNum; ++diag)
        compute_result(diag, similarity_matrix, cur_diag, prev_diag, prev_prev_diag, vectorNumR, maxDiagLen, N);
    
    //* Free ladder
    free_cur:
        free(cur_diag);
    free_prv_prv:
        free(prev_prev_diag);
    free_prv:
        free(prev_diag);
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

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("%s <Query Size N> <DataBase Size M>\n", argv[0]);
		return EXIT_FAILURE;
	}
    printf("Starting Local Alignment Code \n");
	fflush(stdout);
    f = stdout;
	
    /* Typically, M >> N */
	long int N = 256;//atoi(argv[1]); 
    long int M = 256;//atoi(argv[2]);
    long int cols_count = N * 3;

    char *query = ""; //(char*) malloc(sizeof(char) * cols_count);
	char *database = ""; //(char*) malloc(sizeof(char) * M);
	int *similarity_matrix = (int*) malloc(sizeof(int) * (cols_count * M));
	int *max_index = (int *) malloc(sizeof(int));
	// fillRandom(query, N);
	// fillRandom(database, M);

    double start, end;
    start = omp_get_wtime();
    compute_matrices(query, database, max_index, similarity_matrix, NULL, N, M);
	end = omp_get_wtime();

#ifdef ENABLE_TEST_OUTPUT
	printSimilarityMatrix(similarity_matrix, query, database, N, M);
#endif

    printf("max index is in position (%ld, %ld) \n", max_index[0]/(N + 1), max_index[0]%(N+1));
	printf("execution time of LSAL SW algorithm is %f sec \n", end - start);

	free(similarity_matrix);
	free(max_index);
	free(query);
	free(database);

    // printf("cnt_ops=%d, cnt_bytes=%d\n", cnt_ops, cnt_bytes);
	return EXIT_SUCCESS;
}