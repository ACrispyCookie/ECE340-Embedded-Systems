#include <emmintrin.h> // SSE2
#include <omp.h>       // OpenMP
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

void compute_matrices(
    const char *string1, const char *string2,
    int *max_index, int *similarity_matrix, short *direction_matrix, const int N, const int M)
{
    *max_index = N + 1;

    __m128i vZero = _mm_setzero_si128();
    __m128i vMatch = _mm_set1_epi16(MATCH);
    __m128i vMismatch = _mm_set1_epi16(MISS_MATCH);
    __m128i vGapOpen = _mm_set1_epi16(GAP_d);
    __m128i vGapExtend = _mm_set1_epi16(GAP_i);

    // Round N up to next multiple of SIMD_WIDTH
    int padded_N = ((N + SIMD_WIDTH - 1) / SIMD_WIDTH) * SIMD_WIDTH;

    // Loop over each column (j), parallelize over row stripes
    for (int j = 0; j < M; ++j) {
        #pragma omp parallel for
        for (int r = 0; r < padded_N; r += SIMD_WIDTH) {
            // Load stripes (one vector) from each row block
            __m128i vScoresNW, vScoresN, vScoresW;
            __m128i vMaxScore = vZero;
            __m128i vDirection = vZero;

            for (int i = 0; i < SIMD_WIDTH; ++i) {
                int idx = (j * N) + r + i;
                if (r + i >= N) break;

                // Load previous cells (dependencies)
                int idxNW = idx - N - 1;
                int idxN = idx - N;
                int idxW = idx - 1;

                short scoreNW = similarity_matrix[idxNW];
                short scoreN = similarity_matrix[idxN];
                short scoreW = similarity_matrix[idxW];

                short match = (string1[r + i] == string2[j]) ? MATCH : MISS_MATCH;

                short fromNW = scoreNW + match;
                short fromN = scoreN + GAP_d;
                short fromW = scoreW + GAP_i;

                short max = fromNW;
                short dir = NORTH_WEST;
                if (fromN > max) {
                    max = fromN;
                    dir = NORTH;
                }
                if (fromW > max) {
                    max = fromW;
                    dir = WEST;
                }

                similarity_matrix[idx] = max;
                direction_matrix[idx] = dir;

                #pragma omp critical
                {
                    if (max > similarity_matrix[*max_index]) {
                        *max_index = idx;
                    }
                }
            }
        }
    }
}
