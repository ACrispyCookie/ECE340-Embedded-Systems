typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;
#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1
#define N 32
#define M 65536

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
extern "C" {
	void compute_matrices(
		const char *string1, const char *string2,
		int *max_index, int *similarity_matrix, short *direction_matrix)
	{

		int index = 0;
		int similarity_index = 0;
		int i = 0;
		int j = 0;

		// Following values are used for the N, W, and NW values wrt. similarity_matrix[i]
		int north_value = 0;
		int west_value = 0;
		int northwest_value = 0;
		int max_value = 0;

		short match;
		short direction;
		*max_index = N + 1;

		//Here the real computation starts. Place your code whenever is required. 
		// Scan the N*M array row-wise starting from the second row.

		for (i = 0; i < M; i++) {
			index = i * N;
			similarity_index = index + N + i + 2;
			for (j = 0; j < N; j++, index++, similarity_index++) {
				max_value = 0;
				direction = CENTER;

				match = string1[j] == string2[i] ? MATCH : MISS_MATCH;
				northwest_value = similarity_matrix[similarity_index - N - 2] + match;
				north_value = similarity_matrix[similarity_index - N - 1] + GAP_d;
				west_value = similarity_matrix[similarity_index - 1] + GAP_i;

				if (northwest_value > max_value) {
					max_value = northwest_value;
					direction = NORTH_WEST;
				}

				if (north_value > max_value) {
					max_value = north_value;
					direction = NORTH;
				}

				if (west_value > max_value) {
					max_value = west_value;
					direction = WEST;
				}

				similarity_matrix[similarity_index] = max_value;
				direction_matrix[index] = direction;
				if (max_value > similarity_matrix[*max_index])
					*max_index = similarity_index;
			}
		}
	}  // end of function
}
