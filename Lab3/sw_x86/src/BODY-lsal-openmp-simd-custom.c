#include <omp.h>
#include <immintrin.h>

#define CORE_COUNT 6
#define SIMD_WIDTH_INT 8
#define SIMD_WIDTH_CHAR 32

#define MIN(a,b) (((a)<(b))?(a):(b))
#define DIAGONAL_LENGTH(d, N, M) ((d) < (N) && (d) < (M) ? (d) + 1 : (N) + (M) - (d) - 1)


/************************************************************************/

int *aDiagFlatTransform(int *matrix, int matrixSizeN, int matrixSizeM) {
    int totalDiags = matrixSizeN + matrixSizeM - 1;
    int largestDiagLen = MIN(matrixSizeN, matrixSizeM);
    
    int *result = (int *)malloc(totalDiags * largestDiagLen * sizeof(int));
    if (!result) return NULL;

    memset(result, 0, totalDiags * largestDiagLen * sizeof(int));

    // Fill in anti-diagonal elements row by row
    for (int d = 0; d < totalDiags; ++d) {
        int idx = 0;
        for (int i = 0; i < matrixSizeN; ++i) {
            int j = d - i;
            if (j >= 0 && j < matrixSizeM) {
                result[d * largestDiagLen + idx++] = matrix[i * matrixSizeM + j];
            }
        }
    }
    
    return result;
}

//* In a perfect world MM and C should return at about the same time.
//* tweak allowed threads for each until load is balanced. 

void compute_missmatch(int diag, int vectorNum, int *similarity_matrix, const int maxThrAllowed, __m256i* vectors, const char *string1, const char *string2, const int N, const int M) {
    #pragma omp parallel num_threads(maxThrAllowed)
    {
        #pragma omp for schedule(static) private(str1_vec, str2_vec, cmps_vec, result, result32_1, result32_2)
        for (int i = 0, simIdx = 0; i < vectorNum; ++i, simIdx += 32) {
            #ifdef TRANSFORMED_MATRIX
                __m256i str1_vec = _mm256_loadu_si256((__m256i*)&string1[simIdx]);
                __m256i str2_vec = _mm256_loadu_si256((__m256i*)&string2[diag << 5]);
                __m256i cmps_vec = _mm256_cmpeq_epi8(str1_vec, str2_vec);
                
                // Set 'match' value (2), 'mismatch' value (-1)
                __m256i match_val = _mm256_set1_epi8(2);
                __m256i mismatch_val = _mm256_set1_epi8(-1);
                
                __m256i result = _mm256_blendv_epi8(mismatch_val, match_val, cmps_vec);

                // Transform vector from char to ints     
                //* _mm256_castsi256_si128 gets first 128 bits (16 bytes) 
                //* _mm256_cvtepi8_epi32 converts to char to int basically      
                __m256i result32_1 = _mm256_cvtepi8_epi32(_mm256_castsi256_si128(result));
                __m256i result32_2 = _mm256_cvtepi8_epi32(_mm256_extracti128_si256(result, 1)); //* same as above instead get last 128 bits
                
                // Store to matrix
                _mm256_storeu_si256((__m256i*)&similarity_matrix[simIdx], result32_1);
                _mm256_storeu_si256((__m256i*)&similarity_matrix[simIdx + 16], result32_2);
            #else
                // do the MissMatch calculation //* Like above but Storing is prob diff. Remove same stuff from ifdef block.
            #endif
        }
    }
}

void compute_result(int diag, int vectorNum, int *similarity_matrix, const int maxThrAllowed, __m256i* vectors, const int N, const int M) {
    // prev_diag = cur_diag

    // Vectorization 

    // create vZero
    // set 0 diag cur starts from 1 diag
    // pragma for (static) //* (This will create the threads)
    #pragma omp parallel for schedule(static) num_threads(maxThrAllowed)
    for (int i = 0; i < vectorNum; i++) {
        // vector logic here
        // v1 = sub(vectors[i], 1)
        // v2 = sub(sll(vectors[i],1), 1)
        // vCur = add(vectors[i+1], v3)
        // max all vectors

        //* var is either N*diag + i if similarity is the transformed array
        //* or something a little more complex cant be bothered rn :D
        // set *similarity_matrix[var]
    }
}

void compute_matrices(
    const char *string1, const char *string2,
    int *max_index, int *similarity_matrix,
    short *direction_matrix, const int N, const int M)
{
    short maxThreadC = CORE_COUNT * 2 / 3; // Trial and error    
 
    int diagResult = 0;
    int diagMissMatch = 0;
    int diagLen = MIN(N, M);
    int diagNum = N + M - 1;
    int vectorNumR = (diagLen + SIMD_WIDTH_INT - 1) / SIMD_WIDTH_INT;
    int vectorNumMM = (diagLen + SIMD_WIDTH_CHAR - 1) / SIMD_WIDTH_CHAR;
    
    omp_set_nested(1); // Must stay here //! Do not move inside #pragmas

    __m256i* vectorsMM = (__m256i*)_mm_malloc(vectorNumMM * sizeof(__m256i), 32);
    if (vectorsMM == NULL) {
        printf("ALLOC_ERROR for vectorsMM");
        goto MM_ALLOC_ERROR;
    }
        
    __m256i* vectorsR = (__m256i*)_mm_malloc(vectorNumR * sizeof(__m256i), 32);
    if (vectorsR == NULL) {
        printf("ALLOC_ERROR for vectorsR");
        goto R_ALLOC_ERROR;
    }

    #pragma omp parallel num_threads(2) shared(diagResult, diagMissMatch)
    {
        int diagC, diagMM;
        int tid = omp_get_thread_num();

        if (!tid) {
            for (int diag = 0; diag < diagNum; ++diag) {
                while (1) {
                    #pragma omp atomic read
                    diagC = diagResult;
                    #pragma omp atomic read
                    diagMM = diagMissMatch;

                    if (diagC < diagMissMatch) 
                        break;

                    #pragma omp taskyield
                }
                
                compute_result(diag, vectorNumR, similarity_matrix, maxThreadC, vectorsR, N, M);

                #pragma omp atomic
                ++diagResult;
            } 
        } else if (tid == 1) {
            for (int diag = 0; diag < diagNum; ++diag) {
                compute_missmatch(diag, vectorNumMM, similarity_matrix, CORE_COUNT - maxThreadC, vectorsMM, string1, string2, N, M);
                
                #pragma omp atomic
                ++diagMissMatch;
            }
        }
    }

    //* Free Ladder
    R_ALLOC_ERROR: 
        _mm_free(vectorsR);

    MM_ALLOC_ERROR: 
        _mm_free(vectorsMM);
}
