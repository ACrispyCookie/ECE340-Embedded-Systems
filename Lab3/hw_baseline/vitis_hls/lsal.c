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

const short GAP_i = -1;
const short GAP_d = -1;
const short MATCH = 2;
const short MISS_MATCH = -1;
const short CENTER = 0;
const short NORTH = 1;
const short NORTH_WEST = 2;
const short WEST = 3;
// static long int cnt_ops=0;
// static long int cnt_bytes=0;

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
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix, int N, int M)
{

    int index = 0;
    int i = 0;
	int j = 0;
	int match = 0;
	int test_val = 0;
	short dir = CENTER;

    // Following values are used for the N, W, and NW values wrt. similarity_matrix[i]
    int north = 0;
	int west = 0;
	int northwest = 0;
	int max_value = 0;
	*max_index = 0;

	//Here the real computation starts. Place your code whenever is required. 
	// Scan the N*M array row-wise starting from the second row.
    for(index = N; index < N*M; index++) {
		i = index % N; // column index
		j = index / N; // row index
   	    
		if (i == 0) {  
			// first column. 
			northwest = 0;
			west = 0;
		} else {
			northwest = similarity_matrix[index - N - 1];
			west = similarity_matrix[index - 1];
		}

		north = similarity_matrix[index - N]; 
		match = ( string1[i] == string2[j] ) ? MATCH : MISS_MATCH; 

		test_val = northwest + match;
		if (test_val > max_value) {
			max_value = test_val;
			dir = NORTH_WEST;
		}
		
		test_val = north + GAP_d;
		if (test_val > max_value) {
			max_value = test_val;
			dir = NORTH;
		}

		test_val = west + GAP_i;
		if (test_val > max_value) {
			max_value = test_val;
			dir = WEST;
		}

		similarity_matrix[index] = max_value;
		direction_matrix[index] = dir;

		if (max_value > similarity_matrix[*max_index])
			*max_index = index;
		max_value = 0;
		dir = CENTER;
	}   // end of for-loop
}  // end of function
