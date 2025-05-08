#include <omp.h>
#include <immintrin.h>

#define CORE_COUNT 6
#define SIMD_WIDTH_INT 8
#define SIMD_WIDTH_CHAR 32
#define GAP_i -1


#define MIN(a,b) (((a)<(b))?(a):(b))

/************************************************************************/

// TODO IDEA:
//! Maybe try using the transformed array and as the algorithm does
//! each diag transform the 'done' elements (those above the 'diag' (current antidiag variable))
//! Win -> fast get_antidiag, map_antidiag_to_matrix ?

/*
 * This func shifts all elements of a vector 1 element to the left
 ? Returns: Shifted vector
 ! Note: Doesn't change given vector
*/
__m256i _mm256_shift_left_1_epi32(__m256i v) {
    // Shift indices: move lanes 1–7 into 0–6, last lane to zero
    __m256i indices = _mm256_set_epi32(
        255, 7, 6, 5, 4, 3, 2, 1
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
        __m256i str1_vec, str2_vec, cmps_vec, match_val, mismatch_val, result, result32_1, result32_2;
        //* Set for to static (same workload amongst threads) 
        #pragma omp for schedule(static) private(str1_vec, str2_vec, cmps_vec, result, result32_1, result32_2)
        for (int j = 0; j < M; j++) {
            for (int idx = 0; idx < N; idx += SIMD_WIDTH_CHAR) {
                str1_vec = _mm256_loadu_si256((__m256i*)&string1[idx]);
                str2_vec = _mm256_set1_epi8(string2[j]);
                cmps_vec = _mm256_cmpeq_epi8(str1_vec, str2_vec);
                
                // Set values for blendv //* Sets value for packed int of length (epiX -> epi8) 8bits           
                match_val = _mm256_set1_epi8(2);
                mismatch_val = _mm256_set1_epi8(-1);
                
                // Calculation &Alignments //* Calculate vector with missmatch values
                result = _mm256_blendv_epi8(mismatch_val, match_val, cmps_vec);
                result32_1 = _mm256_cvtepi8_epi32(_mm256_castsi256_si128(result)); //* Align first 16 bytes for 32bit integer
                result32_2 = _mm256_cvtepi8_epi32(_mm256_extracti128_si256(result, 1)); //* Align last 16 bytes for 32bit integer
    
                __mm256_storeu_si256((__m256i*)&similarity_matrix[j * M + idx], result32_1);
                __mm256_storeu_si256((__m256i*)&similarity_matrix[j * M + idx + 16], result32_2);
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
        prev_prev_vec = _mm256_loadu_si256((__m256i*)&prev_prev_diag[i << 2]);
        prev_vec = _mm256_loadu_si256((__m256i*)&prev_diag[i << 2]);
        cur_vec = _mm256_loadu_si256((__m256i*)&cur_diag[i << 2]);
        zero_vec = _mm256_setzero_si256();

        // Setup vectors //* Perform operations -1 for W and N and += Match for NW
        prev_vec = _mm256_add_epi32(prev_vec, _mm256_set1_epi32(GAP_i));
        prev_shifted_vec = _mm256_shift_left_1_epi32(prev_vec);
        prev_prev_vec = _mm256_add_epi32(prev_prev_vec, cur_vec); //* cur is mm vector

        // Find max
        max_vec = _mm256_max_epi32(prev_vec, prev_shifted_vec);
        max_vec = _mm256_max_epi32(max_vec, prev_prev_vec);
        max_vec = _mm256_max_epi32(max_vec, zero_vec);

        // Write to matrix //* Write the calculated values back to the similarity_matrix 
        _mm256_storeu_si256((__m256i*)&cur_diag[i << 2], max_vec);

        // TODO Maybe save in temp array here first and call this outside of the loop
        map_antidiag_to_matrix(similarity_matrix, cur_diag, diag, maxDiagLen, N);
    }
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
