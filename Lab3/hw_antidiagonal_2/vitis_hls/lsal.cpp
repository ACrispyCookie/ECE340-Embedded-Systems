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
#define DIAGONAL_LENGTH(d, max_dim, min_dim) ((d) >= (max_dim) ? (min_dim) + (max_dim - d) : (d < min_dim ? d : min_dim))

extern "C" {
	void compute_matrices(
		const char *string1, const char *string2,
		int *max_index, short *direction_matrix)
	{
		unsigned short int row, k;
		unsigned int diag;

		unsigned short int max_index_buf = 0, new_val = 0;
		int cond_north, cond_west, cond_zero;
		unsigned int diagNum = M + N - 1;

		unsigned short int *prev_prev_diag, *prev_diag, *cur_diag;

		// This needs complete partitioning
		unsigned short int diagonal_2[MAX_DIAG_LEN + DIAG_PAD] = {0};
		unsigned short int diagonal_1[MAX_DIAG_LEN + DIAG_PAD] = {0};
		unsigned short int diagonal_0[MAX_DIAG_LEN + DIAG_PAD] = {0};

		prev_prev_diag = diagonal_2;
		prev_diag = diagonal_1;
		cur_diag = diagonal_0;

		unsigned short int max_vals_buf[MAX_DIAG_LEN] = {0};
		unsigned short int max_idx_buf[MAX_DIAG_LEN] = {0};

		#pragma HLS ARRAY_PARTITION variable=max_vals_buf complete dim=1
		#pragma HLS ARRAY_PARTITION variable=max_idx_buf complete dim=1

		#pragma HLS ARRAY_PARTITION variable=diagonal_2 dim=1 complete //factor=16 cyclic
		#pragma HLS ARRAY_PARTITION variable=diagonal_1 dim=1 complete //factor=16 cyclic
		#pragma HLS ARRAY_PARTITION variable=diagonal_0 dim=1 complete //factor=16 cyclic

		unsigned short direction_buf[MAX_DIAG_LEN];
		#pragma HLS ARRAY_PARTITION variable=direction_buf dim=1 complete

		char query_buf[N];
		char database_buf[PADDED_M];
		#pragma HLS BIND_STORAGE variable=query_buf type=ram_t2p impl=bram
		#pragma HLS BIND_STORAGE variable=database_buf type=ram_t2p impl=bram
		#pragma HLS ARRAY_PARTITION variable=query_buf dim=1 factor=16 cyclic 	   // 16 will enforce II >= 2 This is okay because
		#pragma HLS ARRAY_PARTITION variable=database_buf dim=1 factor=16 cyclic
		MEM1: memcpy(query_buf, string1, N * sizeof(char));
		MEM2: memcpy(database_buf, string2, PADDED_M * sizeof(char));

		int north_value = 0, west_value = 0, northwest_value = 0;
		int max_value = 0, curr_max_value = 0;
		short match = 0, direction = 0;

		Diagonals:
		for (diag = 0; diag < diagNum; ++diag) {
			#pragma HLS PIPELINE
			int db_idx = diag + N - 1;
			Rows:
			for (row = 0; row < MAX_DIAG_LEN; ++row) { // For non-padded version replace MAX_DIAG_LEN with DIAGONAL_LENGTH(diag, M, N)
				#pragma HLS UNROLL
				match = (query_buf[row] == database_buf[db_idx - row]) ? MATCH : MISS_MATCH;

				west_value = prev_diag[row] + GAP_d;
				north_value = prev_diag[row + 1] + GAP_i;
				northwest_value = prev_prev_diag[row] + match;

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

				cur_diag[row + DIAG_PAD] = max_value;
				direction_buf[row] = direction;

				unsigned short int buf_max = max_vals_buf[row];
				if (max_value > buf_max) {
					max_vals_buf[row] = max_value;
					max_idx_buf[row] = diag;
				}
			}

			// Move diagonal indexes sequence.
			switch (diag % 3) {
				case 0: {
					cur_diag = diagonal_2;
					prev_diag = diagonal_0;
					prev_prev_diag = diagonal_1;
					break;
				}
				case 1: {
					cur_diag = diagonal_1;
					prev_diag = diagonal_2;
					prev_prev_diag = diagonal_0;
					break;
				}
				default: {
					cur_diag = diagonal_0;
					prev_diag = diagonal_1;
					prev_prev_diag = diagonal_2;
				}
			}

			// Set directional matrix (in RAM)
			// This need reshaping maybe
			MEM3: memcpy(&direction_matrix[diag * MAX_DIAG_LEN], direction_buf, MAX_DIAG_LEN * sizeof(short));
		}

		// Tree-like reduction: find max
		unsigned short int temp_max_val = 0;
		unsigned short int max_index_temp = 0;
		unsigned short int max_diag_index_temp = 0;

		ReduceMax:
		for (k = 0; k < MAX_DIAG_LEN; k++) {
			#pragma HLS UNROLL
			int val = max_vals_buf[k];
			if (val > temp_max_val) {
				temp_max_val = val;
				max_index_temp = k;
				max_diag_index_temp = max_idx_buf[k];
			}
		}

		// if might be better here because of *
		*max_index = (max_diag_index_temp - max_index_temp) * MAX_DIAG_LEN + max_index_temp;
	}
}


