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

#define N 32
#define M 65536
#define PADDED_M (M + 2 * (N - 1))

#define CENTER 0
#define NORTH_WEST 1
#define NORTH 2
#define WEST 3
#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define DIAG_PAD 1
#define MAX_DIAG_LEN MIN(N, M)
#define DIAGONAL_LENGTH(d, max_dim, min_dim) ((d) >= (max_dim) ? (min_dim) + (max_dim - d) : (d < min_dim ? d : min_dim))

extern "C" {
	void compute_matrices(
		const char *string1, const char *string2,
		int *max_index, short *direction_matrix)
	{
		int row, k, diag;

		int diagNum = M + N - 1;

		int diagonal_2[MAX_DIAG_LEN + DIAG_PAD] = {0};
		int diagonal_1[MAX_DIAG_LEN + DIAG_PAD] = {0};
		int diagonal_0[MAX_DIAG_LEN + DIAG_PAD] = {0};

		#pragma HLS ARRAY_PARTITION variable=diagonal_2 dim=1 complete
		#pragma HLS ARRAY_PARTITION variable=diagonal_1 dim=1 complete
		#pragma HLS ARRAY_PARTITION variable=diagonal_0 dim=1 complete

		int max_vals_buf[MAX_DIAG_LEN] = {0};
		int max_idx_buf[MAX_DIAG_LEN] = {0};

		#pragma HLS ARRAY_PARTITION variable=max_vals_buf complete dim=1
		#pragma HLS ARRAY_PARTITION variable=max_idx_buf complete dim=1

		short direction_buf[MAX_DIAG_LEN] = {0};
		#pragma HLS BIND_STORAGE variable=direction_buf type=ram_t2p impl=bram
		#pragma HLS ARRAY_PARTITION variable=direction_buf dim=1 factor=16 cyclic

		char query_buf[N] = {0};
		char database_buf[PADDED_M] = {0};
		#pragma HLS BIND_STORAGE variable=query_buf type=ram_t2p impl=bram
		#pragma HLS BIND_STORAGE variable=database_buf type=ram_t2p impl=bram
		#pragma HLS ARRAY_PARTITION variable=query_buf dim=1 factor=16 cyclic 	   // 16 will enforce II >= 2 This is okay because
		#pragma HLS ARRAY_PARTITION variable=database_buf dim=1 factor=16 cyclic
		MEM1: memcpy(query_buf, string1, N * sizeof(char));
		memcpy(database_buf, string2, PADDED_M * sizeof(char));

		int north_value, west_value, northwest_value;
		int max_value, match;

		short direction = 0;

		Diagonals:
		for (diag = 0; diag < diagNum; ++diag) {
			#pragma HLS PIPELINE
			Rows:
			for (row = 0; row < MAX_DIAG_LEN; ++row) {
				#pragma HLS UNROLL
				match = (query_buf[row] == database_buf[MAX_DIAG_LEN + diag - 1 - row]) ? MATCH : MISS_MATCH;

				west_value = diagonal_1[row] + GAP_d;
				north_value = diagonal_1[row + 1] + GAP_i;
				northwest_value = diagonal_2[row] + match;

				max_value = northwest_value;
				direction = NORTH_WEST;

				if (north_value > max_value) {
					max_value = north_value;
					direction = NORTH;
				}

				if (west_value > max_value) {
					max_value = west_value;
					direction = WEST;
				}

				if (max_value <= 0) {
					max_value = 0;
					direction = CENTER;
				}

				diagonal_0[row + DIAG_PAD] = max_value;
				direction_buf[row] = direction;

				if (max_value > max_vals_buf[row]) {
					max_vals_buf[row] = max_value;
					max_idx_buf[row] = diag;
				}
			}
			memcpy(diagonal_2, diagonal_1, (N + DIAG_PAD) * sizeof(int));
			memcpy(diagonal_1, diagonal_0, (N + DIAG_PAD) * sizeof(int));

			MEM3: memcpy(&direction_matrix[diag * MAX_DIAG_LEN], direction_buf, MAX_DIAG_LEN * sizeof(short));
		}

		int temp_max_val = 0;
		int max_index_temp;
		int max_diag_index_temp;

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

