#include "lsal.h"
#include <string.h>
#include <ap_int.h>

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

#define DIAG_PAD 1
#define MAX_DIAG_LEN MIN(N, M)
#define DIAG_ARRAY_LEN (3 * MAX_DIAG_LEN + 3)
#define DIAGONAL_LENGTH(d, max_dim, min_dim) ((d) >= (max_dim) ? (min_dim) + (max_dim - d) : (d < min_dim ? d : min_dim))

extern "C" {
	void compute_matrices(
		const char *string1, const char *string2,
		int *max_index, short *direction_matrix)
	{
		int diag, row, k;
		int max_index_temp = 0;
		int max_index_buf = 0, new_val = 0;
		int cond_north, cond_west, cond_zero;
		int diagNum = M + N - 1;
	
		// These are offsets for the diagonals these will round robin.
		int prev_prev_idx = 0, prev_idx = MAX_DIAG_LEN + 1, cur_idx = (MAX_DIAG_LEN << 1) + 2, swap_idx = 0;
		
		// This needs complete partitioning
		int diagonals[DIAG_ARRAY_LEN];
		memset(diagonals, 0, DIAG_ARRAY_LEN * sizeof(int));
//		#pragma HLS ARRAY_PARTITION variable=diagonals dim=1 factor=16 cyclic

//		#pragma HLS ARRAY_PARTITION variable=diag_max_val_buf dim=1 complete
		//#pragma HLS ARRAY_PARTITION variable=direction_buf dim=1 complete
		int diag_max_val_buf[MAX_DIAG_LEN];
		short direction_buf[MAX_DIAG_LEN];

		char query_buf[N];
		char database_buf[PADDED_M];
		memcpy(query_buf, string1, N * sizeof(char));
		memcpy(database_buf, string2, PADDED_M * sizeof(char));

		//#pragma HLS ARRAY_RESHAPE variable=query_buf dim=1 factor=4 block
		//#pragma HLS ARRAY_RESHAPE variable=database_buf dim=1 factor=4 block

		int north_value = 0, west_value = 0, northwest_value = 0;
		int max_value = 0, curr_max_value = 0;
		short match = 0, direction = 0;
		*max_index = 0;

		Diagonals:
		for (diag = 0; diag < diagNum; ++diag) {
//			#pragma HLS PIPELINE II=1
			Rows:
			for (int row = 0; row < MAX_DIAG_LEN; ++row) { // For non-padded version replace MAX_DIAG_LEN with DIAGONAL_LENGTH(diag, M, N)
				match = (query_buf[row] == database_buf[diag - row + N - 1]) ? MATCH : MISS_MATCH;

				west_value = diagonals[prev_idx + row] + GAP_d;
				north_value = diagonals[prev_idx + row + 1] + GAP_i;
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
	
				diagonals[cur_idx + row + DIAG_PAD] = max_value;
				direction_buf[row] = direction;
			}

			// Find new max index
			// TODO Find out what to do with max index
			Max: for (k = 0, max_index_temp = -1; k < MAX_DIAG_LEN; k++) {
				new_val = diagonals[cur_idx + k + DIAG_PAD];
				if (curr_max_value  < new_val) {
					curr_max_value  = new_val;
					max_index_temp = k;
				}
			}

			// Move diagonal indexes sequence.
			swap_idx = prev_prev_idx;
			prev_prev_idx = prev_idx;
			prev_idx = cur_idx;
			cur_idx = swap_idx;

			// if might be better here because of *
			if (max_index_temp != -1)
				*max_index = (diag - max_index_temp) * MAX_DIAG_LEN + max_index_temp;

			// Set directional matrix (in RAM)
			// This need reshaping maybe
			memcpy(&direction_matrix[diag * MAX_DIAG_LEN], direction_buf, MAX_DIAG_LEN * sizeof(short));
		}
	}
}


