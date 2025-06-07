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

#define PADDED_M (M + 2 * (N - 1))
#define MIN(a, b) ((a > b) ? b : a)

void print_arr_int(int *arr, int rows, int cols);
void print_arr_short(short *arr, int rows, int cols);
int check_matrix_short(short *ut, int start_offset, int offset_per_row, 
	short *gold, int start_offset_gold, int offset_per_row_gold);
int check_matrix_int(int *ut, int start_offset, int offset_per_row, 
	int *gold, int start_offset_gold, int offset_per_row_gold);
void order_matrix_blocks_int(int *in_matrix, int *out_matrix);
void order_matrix_blocks_short(short *in_matrix, short *out_matrix);
int rand_lim(int limit);
void fillRandom(char* string, int dimension);

int main(int argc, char** argv) {
	// Initialize sizes and allocate arrays
	long similarity_size = N * (PADDED_M + N - 1);
	long direction_size = N * (PADDED_M + N - 1);

	long similarity_size_final = N * PADDED_M;
	long direction_size_final = N * PADDED_M;

	long similarity_size_gold = (N + 1) * (M + 1);
	long direction_size_gold = N * M;

    char *query = (char*) malloc(sizeof(char) * N); //"TGTTACGGTAGTGTTACGGTAG";
	char *database = (char*) malloc(sizeof(char) * M); //"GGTTGACTAGGTTGACTAGGTTGACTA";

	int *similarity_matrix = (int*) malloc(sizeof(int) * similarity_size);
	short *direction_matrix = (short*) malloc(sizeof(short) * direction_size);
	int *max_index = (int *) malloc(sizeof(int));

	int *similarity_matrix_final = (int*) malloc(sizeof(int) * similarity_size_final);
	short *direction_matrix_final = (short*) malloc(sizeof(short) * direction_size_final);

	int *similarity_matrix_gold = (int*) malloc(sizeof(int) * similarity_size_gold);
	short *direction_matrix_gold = (short*) malloc(sizeof(short) * direction_size_gold);
	int *max_index_gold = (int *) malloc(sizeof(int));

	// Initialize arrays
	fillRandom(query, N);
	fillRandom(database, M);
	memset(similarity_matrix, 0, sizeof(int) * similarity_size);
	memset(direction_matrix, 0, sizeof(short) * direction_size);
	memset(similarity_matrix_gold, 0, sizeof(int) * similarity_size_gold);
	memset(direction_matrix_gold, 0, sizeof(short) * direction_size_gold);

	// Call gold and function under test
	compute_matrices_gold(query, database, max_index_gold, similarity_matrix_gold, direction_matrix_gold);
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix);

	// Reshape results
	order_matrix_blocks_int(similarity_matrix, similarity_matrix_final);
	order_matrix_blocks_short(direction_matrix, direction_matrix_final);
    
	// Compare results
	int fail_sim = 0, fail_dir = 0, fail_max = 0;

	fail_sim = check_matrix_int(similarity_matrix_final, (N - 1) * N, 0, similarity_matrix_gold, N + 2, 1);
	fail_dir = check_matrix_short(direction_matrix_final, (N - 1) * N, 0, direction_matrix_gold, 0, 0);
	fail_max = *max_index != *max_index_gold;

	if (fail_sim) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Simulation matrix doesn't match golden output\n");
        fprintf(stdout, "*******************************************\n");
	}

	if (fail_dir) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Direction matrix doesn't match golden output\n");
        fprintf(stdout, "*******************************************\n");
	}

	if (fail_max) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Max index does not match the golden output\n");
        fprintf(stdout, "*******************************************\n");
	}

	if (!fail_sim && !fail_dir && !fail_max) {
		fprintf(stdout, "*******************************************\n");
		fprintf(stdout, "PASS: The output matches the golden output!\n");
		fprintf(stdout, "*******************************************\n");
	}

	// Free everything
	free(query);
	free(database);

	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);

	free(similarity_matrix_gold);
	free(direction_matrix_gold);
	free(max_index_gold);

	free(similarity_matrix_final);
	free(direction_matrix_final);

	return fail_dir || fail_max;
}

void order_matrix_blocks_int(int *in_matrix, int *out_matrix) {
	int num_diag = 0;
	int store_elem = 1;
	int store_index;
	int tmp_i = 0;
	int tmp_j = 0;
	int i = 0;
	int j;

	for(i = 0; i<(PADDED_M + N - 1) * N;){
	//while(store_elem != 0){
		if(num_diag < PADDED_M){
			tmp_j = num_diag;
			tmp_i = 0;
		}else{
			tmp_i = (num_diag + 1) - PADDED_M;
			tmp_j = PADDED_M - 1;
		}

		for(j = 0; j < store_elem; j++){
			store_index = tmp_j * N + tmp_i;
			out_matrix[store_index] = in_matrix[i];
			// printf("stored %d in index %d \n", out_matrix[store_index], store_index);
			tmp_j--;
			tmp_i++;
			i++;
		}
		num_diag++;

		if(num_diag >= PADDED_M){
			store_elem--;
			i = num_diag * N + N - store_elem;
		}else{
			if(store_elem != N){
				store_elem++;
				i = num_diag * N;
			}
		}
	}
}

void order_matrix_blocks_short(short *in_matrix, short *out_matrix) {
	int num_diag = 0;
	int store_elem = 1;
	int store_index;
	int tmp_i = 0;
	int tmp_j = 0;
	int i = 0;
	int j;

	for(i = 0; i<(PADDED_M + N - 1) * N;){
	//while(store_elem != 0){
		if(num_diag < PADDED_M){
			tmp_j = num_diag;
			tmp_i = 0;
		}else{
			tmp_i = (num_diag + 1) - PADDED_M;
			tmp_j = PADDED_M - 1;
		}

		for(j = 0; j < store_elem; j++){
			store_index = tmp_j * N + tmp_i;
			out_matrix[store_index] = in_matrix[i];
			// printf("stored %d in index %d \n", out_matrix[store_index], store_index);
			tmp_j--;
			tmp_i++;
			i++;
		}
		num_diag++;

		if(num_diag >= PADDED_M){
			store_elem--;
			i = num_diag * N + N - store_elem;
		}else{
			if(store_elem != N){
				store_elem++;
				i = num_diag * N;
			}
		}
	}
}

void print_arr_short(short *arr, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			printf("%3d ", arr[i * cols + j]);
		}
		printf("\n");
	}
}

void print_arr_int(int *arr, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			printf("%3d ", arr[i * cols + j]);
		}
		printf("\n");
	}
}

/*
	Checks if the similarity matrix under test is correct.

	Parameters:
	int start_offset - Elements to skip from the start of the matrix under test
	int offset_per_row - Elements to skip for each line on the matrix under test
	int start_offset_gold - Elements to skip from the start of the gold matrix
	int offset_per_row_gold - Elements to skip for each line on the gold matrix
*/
int check_matrix_short(short *ut, int start_offset, int offset_per_row, 
	short *gold, int start_offset_gold, int offset_per_row_gold)
{

	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			int ut_index = start_offset + (i * (N + offset_per_row)) + j;
			int gold_index = start_offset_gold + (i * (N + offset_per_row_gold)) + j;
			if (ut[ut_index] != gold[gold_index]) {
				printf("short: ut_index %d gold_index %d, %d %d\n", ut_index, gold_index, ut[ut_index], gold[gold_index]);
				return 1;
			}
		} 
	}
	return 0;
}

/*
	Checks if the similarity matrix under test is correct.

	Parameters:
	int start_offset - Elements to skip from the start of the matrix under test
	int offset_per_row - Elements to skip for each line on the matrix under test
	int start_offset_gold - Elements to skip from the start of the gold matrix
	int offset_per_row_gold - Elements to skip for each line on the gold matrix
*/
int check_matrix_int(int *ut, int start_offset, int offset_per_row, 
	int *gold, int start_offset_gold, int offset_per_row_gold)
{

	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			int ut_index = start_offset + (i * (N + offset_per_row)) + j;
			int gold_index = start_offset_gold + (i * (N + offset_per_row_gold)) + j;
			if (ut[ut_index] != gold[gold_index]) {
				printf("int: ut_index %d gold_index %d, %d %d\n", ut_index, gold_index, ut[ut_index], gold[gold_index]);
				return 1;
			}
		} 
	}
	return 0;
}

/*
 return a random number in [0, limit].
 */
int rand_lim(int limit) {

	int divisor = RAND_MAX / (limit + 1);
	int retval;

	do {
		retval = rand() / divisor;
	} while (retval > limit);

	return retval;
}

/*
 Fill the string with random values
 */
void fillRandom(char* string, int dimension) {
	//fill the string with random letters..
	static const char possibleLetters[] = "ATCG";

	string[0] = '-';

	int i;
	for (i = 0; i < dimension; i++) {
		int randomNum = rand_lim(3);
		string[i] = possibleLetters[randomNum];
	}
}