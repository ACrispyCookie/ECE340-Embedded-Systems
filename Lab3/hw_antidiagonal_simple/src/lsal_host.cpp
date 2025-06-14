/********************************************************************************
 * The Host code running in the Arm CPU. Make sure to study and understand the OpenCL API code
 * 
 ********************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <CL/opencl.h>
#include <CL/cl_ext.h>

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

/*
 Given an event, this function returns the kernel execution time in ms
 */
double getTimeDifference(cl_event event) {
	cl_ulong time_start = 0;
	cl_ulong time_end = 0;
	double total_time = 0.0f;

	clGetEventProfilingInfo(event,
	CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start,
	NULL);
	clGetEventProfilingInfo(event,
	CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end,
	NULL);
	total_time = time_end - time_start;
	return total_time / 1000000.0; // To convert nanoseconds to milliseconds
}

int load_file_to_memory(const char *filename, char **result) {
	size_t size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		*result = NULL;
		return -1; // -1 means file opening fail
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char *) malloc(size + 1);
	if (size != fread(*result, sizeof(char), size, f)) {
		free(*result);
		return -2; // -2 means file reading fail
	}
	fclose(f);
	(*result)[size] = 0;
	return size;
}

 /***************************************************************************************
  * This is the golden code which runs in the CPU (and is the same code that you developed for x86 / Arm) 
  * It will be used to verify the correct functionality of the HW implementation.  
  * Its usefulness is mainly when you perform software emulation (sw_emu).
  ***************************************************************************************/
void compute_matrices_sw(
    const char *string1, const char *string2,
    int *max_index, int *similarity_matrix, short *direction_matrix)
{
    int i, j;
    long int index = 0;
    long int similarity_index = N + 2;

    int north_value = 0;
    int west_value = 0;
    int northwest_value = 0;
    int max_value = 0;

    short match = 0;
    short direction = 0;
    int max_similarity_index = N + 1;

    for (i = 0; i < M; i++, similarity_index++) {
        northwest_value = 0;
        west_value = 0;
        for (j = 0; j < N; j++, index++, similarity_index++) {
            match = (string1[j] == string2[i]) ? MATCH : MISS_MATCH;

            northwest_value += match;
            north_value = similarity_matrix[similarity_index - N - 1] + GAP_d;

            max_value = northwest_value;
            direction = NORTH_WEST;

            int cond_north = (north_value > max_value);
            max_value = cond_north ? north_value : max_value;
            direction = cond_north ? NORTH : direction;

            int cond_west = (west_value > max_value);
            max_value = cond_west ? west_value : max_value;
            direction = cond_west ? WEST : direction;

            int cond_zero = (0 >= max_value);
            max_value = cond_zero ? 0 : max_value;
            direction = cond_zero ? CENTER : direction;

            similarity_matrix[similarity_index] = max_value;
            direction_matrix[index] = direction;
            
            max_similarity_index = max_value > similarity_matrix[max_similarity_index] ? similarity_index : max_similarity_index;
            northwest_value = north_value - GAP_d;
            west_value = max_value + GAP_i;
        }
    }
    *max_index = max_similarity_index - N - 1 - max_similarity_index / (N + 1);
}  // end of function

void print_arr_char(char *arr, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			printf("%3d ", arr[i * cols + j]);
		}
		printf("\n");
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

void printDirectionMatrix(char *direction_matrix, const int Np, const int Mp) {
	printf("################# Direction Matrix #################\n");
	for (int j = 0; j < Mp; j++) {
		for (int i = 0; i < Np; i++) {
			int index = j * Np + i;

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
			printf("%2s ", s);
		}
		printf("\n");
	}
	printf("####################################################\n");
}

void printDirectionMatrix_short(short *direction_matrix, const int Np, const int Mp) {
	printf("################# Direction Matrix #################\n");
	for (int j = 0; j < Mp; j++) {
		for (int i = 0; i < Np; i++) {
			int index = j * Np + i;

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
			printf("%2s ", s);
		}
		printf("\n");
	}
	printf("####################################################\n");
}

/*
	Checks if the similarity matrix under test is correct.

	Parameters:
	int start_offset - Elements to skip from the start of the matrix under test
	int offset_per_row - Elements to skip for each line on the matrix under test
	int start_offset_gold - Elements to skip from the start of the gold matrix
	int offset_per_row_gold - Elements to skip for each line on the gold matrix
*/
int check_direction(short *ut, short *gold)
{

	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			int ut_row = i + j;
			int ut_col = j;
			int ut_index = ut_row * N + ut_col;
			int gold_index = i * N + j;
			if (ut[ut_index] != gold[gold_index]) {
				printf("short: ut_index %d gold_index %d, %d %d\n", ut_index, gold_index, ut[ut_index], gold[gold_index]);
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
	int i;

	for (i = 0; i < dimension; i++) {
		int randomNum = rand_lim(3);
		string[i] = possibleLetters[randomNum];
	}

}

/*******************************************************************************
 *   Host program running on the Arm CPU. 
 *   The code is written using the OpenCL API. 
 *   We have provided multiple comments for you to understand where each thing  
*******************************************************************************/
int main(int argc, char** argv) {
	printf("starting HOST code \n");
	fflush(stdout);
	int err;                  

    if (argc != 2) {
		printf("%s <input xclbin file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	cl_platform_id platform_id;         // platform id
	cl_device_id device_id;             // compute device id
	cl_context context;                 // compute context
	cl_command_queue commands;          // compute command queue
	cl_program program;                 // compute program
	cl_kernel kernel;                   // compute kernel

	char cl_platform_vendor[1001];
	char cl_platform_name[1001];

	cl_mem input_query;
	cl_mem input_database;
	cl_mem output_similarity_matrix;
	cl_mem output_direction_matrix;
	cl_mem output_max_index;

	int similarity_size_sw = (N + 1) * (M + 1);
	int direction_size_sw = N * M;

	char *query = (char*) malloc(sizeof(char) * N);
	char *database = (char*) malloc(sizeof(char) * PADDED_M);

	short *direction_matrix = (short*) malloc(sizeof(short) * N * PADDED_M);
	int *max_index = (int *) malloc(sizeof(int));

	int * similarity_matrix_sw = ( int *) malloc(
			sizeof( int) * similarity_size_sw);
	short * direction_matrix_sw = ( short*) malloc(
			sizeof( short) * direction_size_sw);
	int * max_index_sw = ( int *) malloc(
				sizeof( int));

	printf("array defined! \n");
    fflush(stdout);

	fillRandom(query, N);
	memset(database, 'P', sizeof(char) * PADDED_M);
	fillRandom(database + N - 1, M);

	memset(direction_matrix, 0, sizeof(short) * N * PADDED_M);
	memset(similarity_matrix_sw, 0, sizeof(int) * similarity_size_sw);


/**********************************************
 * 			Xilinx OpenCL Initialization
 *
 * We must follow specific steps to get the necessary
 * information and handlers, in order to be able
 * to use the available accelerator device (FPGA).
 * After every step, we always check for any errors
 * that might have occured. In case of error, the
 * program aborts and exits immediately.
 * *********************************************/

  /**************************************************
	* Step 1:
   * Get available OpenCL platforms and devices.
   * In our case, it is a Xilinx FPGA device.
   * If the underlying platform has other accelerators
   * available, we could use them too (e.g. GPU, CPU).
	**************************************************/
	printf("GET platform \n");
	err = clGetPlatformIDs(1, &platform_id, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to find an OpenCL platform!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("GET platform vendor \n");
	err = clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, 1000,
			(void *) cl_platform_vendor, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: clGetPlatformInfo(CL_PLATFORM_VENDOR) failed!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("CL_PLATFORM_VENDOR %s\n", cl_platform_vendor);
	printf("GET platform name \n");
	err = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 1000,
			(void *) cl_platform_name, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: clGetPlatformInfo(CL_PLATFORM_NAME) failed!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("CL_PLATFORM_NAME %s\n", cl_platform_name);

	// Connect to a compute device
	//
	int fpga = 1;

	printf("get device FPGA is %d  \n", fpga);
	err = clGetDeviceIDs(platform_id,
			fpga ? CL_DEVICE_TYPE_ACCELERATOR : CL_DEVICE_TYPE_CPU, 1,
			&device_id, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to create a device group!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	/*********************************************
	 * Step 2 : Create Context
	 *********************************************/
	printf("create context \n");
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if (!context) {
		printf("Error: Failed to create a compute context!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

   /*********************************************
	 * Step 3 : Create Command Queue
	 *********************************************/
	printf("create queue \n");
	commands = clCreateCommandQueue(context, device_id,
	CL_QUEUE_PROFILING_ENABLE, &err);
	if (!commands) {
		printf("Error: Failed to create a command commands!\n");
		printf("Error: code %i\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	cl_int binary_status;

	/*********************************************
	 * Step 4 : Load Hardware Binary File (*.xclbin) from disk
	 **********************************************/
	unsigned char *kernelbinary;
	char *xclbin = argv[1];
	printf("loading %s\n", xclbin);
	int n_i = load_file_to_memory(xclbin, (char **) &kernelbinary);
	if (n_i < 0) {
		printf("failed to load kernel from xclbin: %s\n", xclbin);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

  /********************************************************
	* Step 5 : Create program using the loaded hardware binary file
	********************************************************/
	size_t n = n_i;
	// Create the compute program from offline
	printf("create program with binary \n");
	program = clCreateProgramWithBinary(context, 1, &device_id, &n,
			(const unsigned char **) &kernelbinary, &binary_status, &err);
	free(kernelbinary);

	if ((!program) || (err != CL_SUCCESS)) {
		printf("Error: Failed to create compute program from binary %d!\n",
				err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	/**************************************************************
	 *  Step 6 for 1 Compute Unit: Create Kernels - the actual handler of the kernel that
    *           we will be using. We first create a program, and then
    *           obtain the kernel handler from the program.
	 **************************************************************/
	printf("build program \n");
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		size_t len;
		char buffer[2048];

		printf("Error: Failed to build program executable!\n");
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
				sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	// Create the compute kernel in the program we wish to run
	//
	printf("create kernel \n");
	kernel = clCreateKernel(program, "compute_matrices", &err);
	if (!kernel || err != CL_SUCCESS) {
		printf("Error: Failed to create compute kernel!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

    /**************************************************************-
    * Step 7 : Create buffers.
    * We do not need to allocate separate memory space (malloc), because
    * on a MPSoC system (e.g. ZCU102 board), we map the memory space that is
    * allocated at clCreateBuffer, to a usable memory space for our
    * host application. We also do not need to use free for any reason.
    * See Xilinx UG1393 for detailed information.
    **************************************************************/
	input_query = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(char) * N,
	NULL, NULL);
	input_database = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(char) * PADDED_M,
	NULL, NULL);
	// output_similarity_matrix = clCreateBuffer(context, CL_MEM_READ_WRITE,
	// 		sizeof(int) * M * N, NULL, NULL);
	output_direction_matrix = clCreateBuffer(context, CL_MEM_READ_WRITE,
			sizeof(short) * N * PADDED_M, NULL, NULL);
	output_max_index = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int),
	NULL, NULL);

	if (!input_query || !input_database
			|| !output_direction_matrix || !output_max_index) {
		printf("Error: Failed to allocate device memory!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

   /**************************************************************
    * Step 8 : Write the Input Data to the Write Buffers of the device memory
    **************************************************************/
	err = clEnqueueWriteBuffer(commands, input_query, CL_TRUE, 0,
			sizeof(char) * N, query, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to write to source array a!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	err = clEnqueueWriteBuffer(commands, input_database, CL_TRUE, 0,
			sizeof(char) * PADDED_M, database, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to write to source array b!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	// err = clEnqueueWriteBuffer(commands, output_similarity_matrix, CL_TRUE, 0,
	// 		sizeof(int) * matrix_size, similarity_matrix, 0, NULL, NULL);
	// if (err != CL_SUCCESS) {
	// 	printf("Error: Failed to write to source array c!\n");
	// 	printf("Test failed\n");
	// 	return EXIT_FAILURE;
	// }

	// err = clEnqueueWriteBuffer(commands, output_direction_matrix, CL_TRUE, 0,
	// 			sizeof(char) * N_hw * M_hw, direction_matrix, 0, NULL, NULL);
	// 	if (err != CL_SUCCESS) {
	// 		printf("Error: Failed to write to source array d!\n");
	// 		printf("Test failed\n");
	// 		return EXIT_FAILURE;
	// 	}

	/**************************************************************
	 * Step 9: Set the arguments to our compute kernel
	 **************************************************************/
	err = 0;
	printf("set arg 0 \n");
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_query);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments 0! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("set arg 1 \n");
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_database);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments 1! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("set arg 2 \n");
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_max_index);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments 2! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	// printf("set arg 3 \n");
	// err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &output_similarity_matrix);
	// if (err != CL_SUCCESS) {
	// 	printf("Error: Failed to set kernel arguments 3! %d\n", err);
	// 	printf("Test failed\n");
	// 	return EXIT_FAILURE;
	// }
	printf("set arg 3 \n");
	err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &output_direction_matrix);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments 3! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	// printf("set arg 5 \n");
	// err |= clSetKernelArg(kernel, 5, sizeof(cl_uint),
	// 		&N_hw);
	// if (err != CL_SUCCESS) {
	// 	printf("Error: Failed to set kernel arguments 5! %d\n", err);
	// 	printf("Test failed\n");
	// 	return EXIT_FAILURE;
	// }
	// printf("set arg 6 \n");
	// err |= clSetKernelArg(kernel, 6, sizeof(cl_uint),
	// 		&M_hw);
	// if (err != CL_SUCCESS) {
	// 	printf("Error: Failed to set kernel arguments 6! %d\n", err);
	// 	printf("Test failed\n");
	// 	return EXIT_FAILURE;
	// }

	
	/**************************************************************
	 * Step 10: Place the Kernel in the Queue for Execution
	 **************************************************************/
	cl_event enqueue_kernel;
	printf("LAUNCH task \n");
	err = clEnqueueTask(commands, kernel, 0, NULL, &enqueue_kernel);

	if (err) {
		printf("Error: Failed to execute kernel! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	clWaitForEvents(1, &enqueue_kernel);

	/**************************************************************
	 * Step 11: Read back the results from the device to verify the output
	 **************************************************************/
	cl_event readMax, readSimilarity, readDirections;
	// err = clEnqueueReadBuffer(commands, output_similarity_matrix, CL_TRUE, 0,
	// 		sizeof(int) * N * M, similarity_matrix, 0, NULL, &readSimilarity);
	// if (err != CL_SUCCESS) {
	// 	printf("Error: Failed to read array! %d\n", err);
	// 	printf("Test failed\n");
	// 	return EXIT_FAILURE;
	// }

	err = clEnqueueReadBuffer(commands, output_direction_matrix, CL_TRUE, 0,
			sizeof(short) * N * PADDED_M, direction_matrix, 0, NULL,
			&readDirections);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to read array! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	err = clEnqueueReadBuffer(commands, output_max_index, CL_TRUE, 0,
			sizeof(int), max_index, 0, NULL, &readMax);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to read array! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	//clWaitForEvents(1, &readSimilarity);
	clWaitForEvents(1, &readDirections);
	clWaitForEvents(1, &readMax);

	double executionTime = getTimeDifference(enqueue_kernel);

   clReleaseMemObject(input_database);
	clReleaseMemObject(input_query);
	clReleaseMemObject(output_direction_matrix);
	clReleaseMemObject(output_max_index);
	// clReleaseMemObject(output_similarity_matrix);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);

	printf(" execution time is %lf ms \n", executionTime);

/**************************************************************
 * Run the same algorithm in the Host Unit and compare for verification
 **************************************************************/

	compute_matrices_sw(query, database + N - 1, max_index_sw, similarity_matrix_sw, direction_matrix_sw);

	// Compare results
	int fail_sim = 0, fail_dir = 0, fail_max = 0;

	fail_sim = 0; 
	fail_dir = check_direction(direction_matrix, direction_matrix_sw);
	fail_max = *max_index != *max_index_sw;

	if (fail_sim) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Simulation matrix doesn't match golden output\n");
        fprintf(stdout, "*******************************************\n");
        print_arr_int(similarity_matrix_sw, M + 1, N + 1);
	}

	if (fail_dir) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Direction matrix doesn't match golden output\n");
        fprintf(stdout, "*******************************************\n");
		fprintf(stdout, "*******************************************\n");
		printf("Similarity golden:\n");
		print_arr_int(similarity_matrix_sw, M + 1, N + 1);
		printf("Direction golden:\n");
		printDirectionMatrix_short(direction_matrix_sw, N, M);
		printf("Direction hardware:\n");
		printDirectionMatrix_short(direction_matrix, N, PADDED_M);
		fprintf(stdout, "*******************************************\n");
	}

	if (fail_max) {
        fprintf(stdout, "*******************************************\n");
        fprintf(stdout, "FAIL: Max index does not match the golden output\n");
        fprintf(stdout, "Golden: %d Under test: %d\n", *max_index_sw, *max_index);
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

	free(direction_matrix);
	free(max_index);

	free(similarity_matrix_sw);
	free(direction_matrix_sw);
	free(max_index_sw);

	return EXIT_SUCCESS;
}
