/* Optimizations:
   - Reuse of already fetched elements used previously
*/

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

typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;

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

void printSimilarityMatrix(FILE *out, int *similarity_matrix, char *query, char *database, const int N, const int M) {
	fprintf(out, "################# Similarity Matrix #################\n");

	//Print query string
	fprintf(out, "     ");
	for (int i = 0; i < N; i++) {
		fprintf(out, "%3c", query[i]);
	}
	fprintf(out, "\n");

	//Print matrix and database
	fprintf(out, "  ");
	for (int j = 0; j < M + 1; j++) {
		if (j > 0)
			fprintf(out, "%c ", database[j - 1]);

		for (int i = 0; i < N + 1; i++) {
			int index = j * (N + 1) + i;
			fprintf(out, "%3d", similarity_matrix[index]);
		}

		fprintf(out, "\n");
	}

	fprintf(out, "#####################################################\n");
}

void printDirectionMatrix(FILE *out, short *direction_matrix, const int N, const int M) {
	fprintf(out, "################# Direction Matrix #################\n");
	for (int j = 0; j < M; j++) {
		for (int i = 0; i < N; i++) {
			int index = j * N + i;

			char *s;
			switch (direction_matrix[index]) {
				case NORTH_WEST:
					s = "NW";
					break;
				case NORTH:
					s = "N";
					break;
				case WEST:
					s = "W";
					break;
				case CENTER:
				default:
					s = "C";
					break;
			}
			fprintf(out, "%2s ", s);
		}
		fprintf(out, "\n");
	}
	fprintf(out, "####################################################\n");
}

/* ******************************************************************/
int main(int argc, char** argv) {
    FILE *out;

	/* Typically, M >> N */
    out = fopen("out.dat","w");
	long int N = 256; 
    long int M = 15000;

    char *query = (char*) malloc(sizeof(char) * N);//"TGTTACGGTAGTGTTACGGTAG";
	char *database = (char*) malloc(sizeof(char) * M);//"GGTTGACTAGGTTGACTAGGTTGACTA";
	int *similarity_matrix = (int*) malloc(sizeof(int) * ((N + 1) * (M + 1)));
	short *direction_matrix = (short*) malloc(sizeof(short) * N * M);
	int *max_index = (int *) malloc(sizeof(int));

	/* Create the two input strings by calling a random number generator */
	fillRandom(query, N);
	fillRandom(database, M);

	memset(similarity_matrix, 0, sizeof(int) * ((N + 1) * (M + 1)));
	memset(direction_matrix, 0, sizeof(short) * N * M);

	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, N, M);

	printSimilarityMatrix(out, similarity_matrix, query, database, N, M);
	printDirectionMatrix(out, direction_matrix, N, M);

    fprintf(out, "max index is in position (%ld, %ld) \n", max_index[0]/(N + 1), max_index[0]%(N+1));
	
	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);
	free(query);
	free(database);
    fclose(out);
    
    printf ("Comparing against output data \n");
    if (system("diff -w out.dat correct_out")) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Output DOES NOT match the golden output\n");
        fprintf(stdout, "*******************************************\n");
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "PASS: The output matches the golden output!\n");
        fprintf(stdout, "*******************************************\n");
        return EXIT_SUCCESS;
    }

    // printf("cnt_ops=%d, cnt_bytes=%d\n", cnt_ops, cnt_bytes);
	return EXIT_SUCCESS;
}
