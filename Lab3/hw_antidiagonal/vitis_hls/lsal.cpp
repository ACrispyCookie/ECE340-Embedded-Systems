#include "lsal.h"
#include <string.h>

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
	int diag, row, k;
	int max_index_temp = 0;
	int max_index_buf = 0, new_val = 0;
	int cond_north, cond_west, cond_zero;
	int diagNum = M + MAX_DIAG_LEN;

	// These are offsets for the diagonals these will round robin.
	int prev_prev_idx = 1, swap_idx = 0, prev_idx = MAX_DIAG_LEN + 2, cur_idx = (MAX_DIAG_LEN << 1) + 3;

	// This needs complete partitioning
	int diagonals[3 * MAX_DIAG_LEN + 3];
	int diag_max_val_buf[MAX_DIAG_LEN] = {0};
	short direction_buf[MAX_DIAG_LEN];

	// char query_buf[MEMCPY_CHAR_BW];
	// char database_buf[MEMCPY_CHAR_BW];

    int north_value = 0, west_value = 0, northwest_value = 0;
	int max_value = 0;

	short match = 0, direction = 0;
	*max_index = 0;

	// memcpy(database_buf, &string2[MAX_DIAG_LEN - i*], MIN(MEMCPY_CHAR_BW >> 1, M)); // TODO string need padding.
	// memcpy(query_buf, string1, MEMCPY_CHAR_BW >> 1);

	for (diag = 0; diag < diagNum + 1; ++diag) {
		// The BW >> 1 allows for II = 1. Else we need more AXI channels.
		for (int row = 0; row < MAX_DIAG_LEN; ++row) { // For non-padded version replace MAX_DIAG_LEN with DIAGONAL_LENGTH(diag, M, N)
			match = (string1[row] == string2[diag]) ? MATCH : MISS_MATCH;

			north_value = diagonals[prev_idx + row] + GAP_i;
			west_value = diagonals[prev_idx + row + 1] + GAP_d;
			northwest_value = diagonals[prev_prev_idx + row] + match;

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

			diagonals[cur_idx + row] = max_value;
			
			diag_max_val_buf[row] = diag + row * MAX_DIAG_LEN;
			direction_buf[row] = direction;
		}

		// Find new max index
		for (k = 0, max_index_temp = -1; k < MAX_DIAG_LEN; k++) {
			new_val = diag_max_val_buf[k];
			if (max_value < new_val) {
				max_value = new_val;
				max_index_temp = k;
			}
		}

		// if might be better here because of *
		if (max_index_temp != -1) *max_index = (diag - max_index_temp) + max_index_temp * MAX_DIAG_LEN;

		// Move diagonal indexes sequence.
		swap_idx = prev_prev_idx;
		prev_prev_idx = prev_idx;
		prev_idx = cur_idx;
		cur_idx = swap_idx;

		// Set directional matrix (in RAM)
		// This need reshaping maybe
		memcpy(&direction_matrix[diag * MAX_DIAG_LEN], direction_buf, MAX_DIAG_LEN);
	}
}

