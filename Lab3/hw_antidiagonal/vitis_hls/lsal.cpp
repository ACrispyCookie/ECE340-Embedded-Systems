#include "lsal.h"

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

void compute_matrices(
	const char *string1, const char *string2,
	int *max_index, short *direction_matrix)
{
	int i, j, k;
	int max_index_temp = 0;
	int max_index_buf = 0;
	int cond_north, cond_west, cond_zero;

	// These are offsets for the diagonals these will round robin.
	int prev_prev_idx = 0, prev_idx = max_diag_length + 2, cur_idx = (max_diag_length << 1) + 3;

	// This needs complete partitioning
	int diagonals[3 * (MAX_DIAG_LEN + 1)];
	int diag_max_val_buf[MAX_DIAG_LEN];
	int database_offset = 0;

	char query_buf[MEMCPY_CHAR_BW];
	char database_buf[MEMCPY_CHAR_BW];

    int north_value = 0, west_value = 0, northwest_value = 0;
	int max_value = 0;

	short match = 0, direction = 0;
	*max_index = N + 1;

	// THESE FOR LOOPS ARE DOGSHIT WE NEED THE DIAGONAL ONES + PADDING ACCOUNTED FOR
	for (i = 0; i < M + MAX_DIAG_LEN; i++) {
		// The BW >> 1 allows for II = 1. Else we need more AXI channels.
		// memcpy(database_buf, &string2[MAX_DIAG_LEN - i*], MIN(MEMCPY_CHAR_BW >> 1, M)); // TODO string need padding.
		for (j = 0; j < N; j++) {
			//memcpy(query_buf, string1, MEMCPY_CHAR_BW >> 1);

			match = (string1[j] == string2[i]) ? MATCH : MISS_MATCH;

			west_value = diagonals[prev_idx + j] + GAP_i;
			north_value = diagonals[prev_idx + j + 1] + GAP_d;
			northwest_value = diagonals[prev_prev_idx + j] + match;

			max_value = northwest_value;
			direction = NORTH_WEST;

			cond_north = (north_value > max_value);
			max_value = cond_north ? north_value : max_value;
			direction = cond_north ? NORTH : direction;

			cond_west = (west_value > max_value);
			max_value = cond_west ? west_value : max_value;
			direction = cond_west ? WEST : direction;

			cond_zero = (0 >= max_value);
			max_value = cond_zero ? 0 : max_value;
			direction = cond_zero ? CENTER : direction;

			diagonals[cur_idx + j] = max_value;
			diag_max_val_buf[j] = (i - MAX_DIAG_LEN) + j * N;
		}
		max_index_temp = -1;
		for (k = 0; k < MAX_DIAG_LEN; k++) {
			new_val = diag_max_val_buf[k];
			if (max_value < new_val) {
				max_value = new_val;
				max_index_temp = k;
			}
		}
		// if (max_index_temp != -1) max_index_buf = ...;
	}
}

