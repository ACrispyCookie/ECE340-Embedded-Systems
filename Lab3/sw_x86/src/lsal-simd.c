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

#define CORE_COUNT 6
#define MIN(a,b) (((a)<(b))?(a):(b))
#define SIMD_WIDTH 8
typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;

/************************************************************************/

//* In a perfect world MM and C should return at about the same time.
//* tweak allowed threads for each until load is balanced. 

void compute_missmatch(int diag, int minSideSize, int *similarity_matrix, const int maxThrAllowed, const int N, const int M) {
    //* Calculate minSideSize between N and M 1 time somewhere outside of this function 
    //* that's why this exists
    const int diagLen = MIN(diag, minSideSize);
    int vectorNum = (diagLen + SIMD_WIDTH - 1) / SIMD_WIDTH;

    #pragma omp parallel num_threads(maxThrAllowed)
    {
        __m256i* vectors = (__m256i*)_mm_malloc(vectorNum * sizeof(__m256i), 32);
        if (vectors == NULL) exit(EXIT_FAILURE);
        
        #pragma omp for schedule(static)
        for (int i = 0; i < vectorNum; ++i) {
            //* var is either N*diag + i if similarity is the transposed array
            //* or something a little more complex cant be bothered rn :D
            #ifdef TRANSFORMED_MATRIX
                // do the MissMatch calculation //* I CANT THINK RN
                // set *similarity_matrix[var] //* easy because a row = the anti-diagonal <- idk its name
            #else
                // do the MissMatch calculation //* I CANT THINK RN
                // set *similarity_matrix[var] //* Weird index stuff
            #endif
        }

        // Free all vectors //* Could do 1 time from main function and pass pointer?
        _mm_free(vectors);
    }

}

void compute_result(int diag, int minSideSize, int *similarity_matrix, const int maxThrAllowed, const int N, const int M) {
    // prev_diag = cur_diag

    //* Calculate minSideSize between N and M 1 time somewhere outside of this function 
    //* that's why this exists
    const int diagLen = MIN(diag, minSideSize);
    int vectorNum = (diagLen + SIMD_WIDTH - 1) / SIMD_WIDTH;
    // Vectorization 

    // create vZero
    // set 0 diag cur starts from 1 diag
    //pragma for (static) //* (This will create the threads)
    #pragma omp parallel for schedule(static) num_threads(maxThrAllowed)
    for (int i = 0; i < vectorNum; i++) {
        // vector logic here
        // v1 = sub(vectors[i], 1)
        // v2 = sub(sll(vectors[i],1), 1)
        // vCur = add(vectors[i+1], v3)
        // max all vectors

        //* var is either N*diag + i if similarity is the transposed array
        //* or something a little more complex cant be bothered rn :D
        // set *similarity_matrix[var]
    }
}

void compute_matrices(
    const char *string1, const char *string2,
    int *max_index, int *similarity_matrix,
    short *direction_matrix, const int N, const int M)
{
    int diagResult = 0;
    int diagMissMatch = 0;

    short maxThreadC = CORE_COUNT * 2 / 3; 

    omp_lock_t mutex;
    omp_init_lock(&mutex);
    omp_set_nested(1); // Must stay here //! Do not move inside #pragmas

    int minSideSize = MIN(N, M);

    #pragma omp parallel num_threads(2) shared(diagResult, diagMissMatch, mutex) 
    {
        int diagC, diagMM;
        int tid = omp_get_thread_num();

        if (!tid) {
            for (int diag = 0; diag < minSideSize; ++diag) {
                while (1) {
                    #pragma omp atomic read
                    diagC = diagResult;
                    #pragma omp atomic read
                    diagMM = diagMissMatch;

                    if (diagC < diagMM) 
                        break;

                    #pragma omp taskyield
                }  
                
                omp_set_lock(&mutex);
                compute_result(diag, minSideSize, similarity_matrix, maxThreadC, N, M);
                omp_unset_lock(&mutex);

                #pragma omp atomic
                ++diagResult;
            } 
        } else if (tid == 1) {
            for (int diag = 0; diag < minSideSize; ++diag) {
                compute_missmatch(diag, minSideSize, similarity_matrix, CORE_COUNT - maxThreadC, N, M);
                
                #pragma omp atomic
                ++diagMissMatch;
            }
        }
    }

    omp_destroy_lock(&mutex);
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
