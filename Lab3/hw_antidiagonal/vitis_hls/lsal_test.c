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
#include "lsal.h"

/************************************************************************/

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

/* ******************************************************************/
int main(int argc, char** argv) {
	/* Typically, M >> N */
	long int similarity_size = (N + 1) * (M + 1);
	long int direction_size = N * M;

    char *query = (char*) malloc(sizeof(char) * N);//"TGTTACGGTAGTGTTACGGTAG";
	char *database = (char*) malloc(sizeof(char) * M);//"GGTTGACTAGGTTGACTAGGTTGACTA";
	int *similarity_matrix = (int*) malloc(sizeof(int) * ((N + 1) * (M + 1)));
	short *direction_matrix = (short*) malloc(sizeof(short) * N * M);
	int *max_index = (int *) malloc(sizeof(int));
	int *similarity_matrix_gold = (int*) malloc(sizeof(int) * ((N + 1) * (M + 1)));
	short *direction_matrix_gold = (short*) malloc(sizeof(short) * N * M);
	int *max_index_gold = (int *) malloc(sizeof(int));

	/* Create the two input strings by calling a random number generator */
	fillRandom(query, N);
	fillRandom(database, M);

	memset(similarity_matrix, 0, sizeof(int) * ((N + 1) * (M + 1)));
	memset(similarity_matrix_gold, 0, sizeof(int) * ((N + 1) * (M + 1)));
	memset(direction_matrix, 0, sizeof(short) * N * M);
	memset(direction_matrix_gold, 0, sizeof(short) * N * M);

	compute_matrices_gold(query, database, max_index_gold, similarity_matrix_gold, direction_matrix_gold);
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix);
    
    printf ("Comparing against gold version \n");
	int fail_sim = 0, fail_dir = 0, fail_max = 0, i = 0;

	for (i = 0; i < similarity_size; i++) {
		if (similarity_matrix[i] != similarity_matrix_gold[i]) {
			fail_sim = 1;
			break;
		}
	}

	if (fail_sim) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Similarity matrix doesn't match golden output\n");
        fprintf(stdout, "*******************************************\n");
	}

	for (i = 0; i < direction_size; i++) {
		if (direction_matrix[i] != direction_matrix_gold[i]) {
			fail_dir = 1;
			break;
		}
	}

	if (fail_dir) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Direction matrix doesn't match golden output\n");
        fprintf(stdout, "*******************************************\n");
	}

	if (*max_index != *max_index_gold) {
		fail_max = 1;
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Max index does not match the golden output\n");
        fprintf(stdout, "*******************************************\n");
	}

	if (!fail_sim && !fail_dir && !fail_max) {
		fprintf(stdout, "*******************************************\n");
		fprintf(stdout, "PASS: The output matches the golden output!\n");
		fprintf(stdout, "*******************************************\n");
	}

	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);
	free(similarity_matrix_gold);
	free(direction_matrix_gold);
	free(max_index_gold);
	free(query);
	free(database);

	return fail_sim || fail_dir || fail_max;
}
