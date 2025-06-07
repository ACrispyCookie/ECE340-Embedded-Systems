#include "lsal.h"

extern "C" {
    void compute_matrices_gold(
        const char *string1, const char *string2,
        int *max_index, int *similarity_matrix, short *direction_matrix)
    {
        int i, j;
        long int index = 0;
        long int similarity_index = N + 2;

        int north_value = 0;
        int west_value = 0;
        int northwest_value = 0;
        int max_value = 0;

        short match = 0;
        short direction = 0;
        int max_similarity_index = N + 1;

        for (i = 0; i < M; i++, similarity_index++) {
            northwest_value = 0;
            west_value = 0;
            for (j = 0; j < N; j++, index++, similarity_index++) {
                match = (string1[j] == string2[i]) ? MATCH : MISS_MATCH;

                northwest_value += match;
                north_value = similarity_matrix[similarity_index - N - 1] + GAP_d;

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
                direction_matrix[index] = direction;
                
                max_similarity_index = max_value > similarity_matrix[max_similarity_index] ? similarity_index : max_similarity_index;
                northwest_value = north_value - GAP_d;
                west_value = max_value + GAP_i;
            }
        }
        *max_index = max_similarity_index - N - 1 - max_similarity_index / (N + 1);
    }  // end of function
}