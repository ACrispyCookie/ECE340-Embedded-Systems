
#include <omp.h>
#define CORE_COUNT 6
#define MIN(a,b) (((a)<(b))?(a):(b))

/************************************************************************/

//* In a perfect world MM and C should return at about the same time.
//* tweak allowed threads for each until load is balanced. 

void compute_missmatch(int diag, int minSideSize, int **similarity_matrix, const int maxThrAllowed, const int N, const int M) {
    //* Calculate minSideSize between N and M 1 time somewhere outside of this function 
    //* that's why this exists
    const int diagLen = MIN(diag, minSideSize);
    int vectorNum = (diagLen + SIMD_WIDTH - 1) / SIMD_WIDTH;
    // vectorization #vectorNum vectors //* maybe too many not enough registers

    //pragma for (static) //* (This will create the threads)
    #pragma omp parallel for schedule(static) num_threads(maxThrAllowed)
    for (int i = 0; i < vectorNum; i++) {
        // vector logic here

        //* var is either N*diag + i if similarity is the transposed array
        //* or something a little more complex cant be bothered rn :D
        // set *similarity_matrix[var]
    }
}

void compute_result(int diag, int minSideSize, int **similarity_matrix, const int maxThrAllowed, const int N, const int M) {
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

                    if (diagC < diagMissMatch) 
                        break;

                    #pragma omp taskyield
                }  
                
                omp_set_lock(&mutex);
                compute_result(diag, minSideSize, &similarity_matrix, maxThreadC, N, M);
                omp_unset_lock(&mutex);

                #pragma omp atomic
                ++diagResult;
            } 
        } else if (tid == 1) {
            for (int diag = 0; diag < minSideSize; ++diag) {
                compute_missmatch(diag, minSideSize, &similarity_matrix, CORE_COUNT - maxThreadC, N, M);
                
                #pragma omp atomic
                ++diagMissMatch;
            }
        }
    }

    omp_destroy_lock(&mutex);
}
