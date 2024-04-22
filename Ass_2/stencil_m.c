#include "stencil.h"
#include <math.h>
#include <string.h>

void exchange_ghost_cells_non_blocking(int id, int procs, int extent, int recv_count, double* local_data, double* extended_data, MPI_Request *request);

int main(int argc, char **argv) {
    if (4 != argc) {
        printf("Usage: stencil input_file output_file number_of_applications\n");
        return 1;
    }

    char *input_name = argv[1];
    char *output_name = argv[2];
    int num_steps = atoi(argv[3]);

    MPI_Init(&argc, &argv);
    int procs, id;
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // Read input file on the root process
    double *input = NULL;
    int num_values = 0;
        if (0 > (num_values = read_input(input_name, &input))) {
                return 2;
        }

    // Broadcast the number of values to all processes
    MPI_Bcast(&num_values, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the number of elements each process will handle
    int recv_count = num_values / procs;

    double *input_MPI = malloc(recv_count * sizeof(double));
    double *output_MPI = malloc(recv_count * sizeof(double));
    double *output = malloc(num_values * sizeof(double));
    
    MPI_Request request[4];
    double h = 2.0 * PI / num_values; 
    const int STENCIL_WIDTH = 5;
    const int EXTENT = STENCIL_WIDTH / 2;
    const double STENCIL[] = {1.0/(12*h), -8.0/(12*h), 0.0, 8.0/(12*h), -1.0/(12*h)};
    double *extended_input = malloc((recv_count + 2 * STENCIL_WIDTH / 2) * sizeof(double));

    // Scatter the input data from the root process to all processes
    MPI_Scatter(input, recv_count, MPI_DOUBLE, input_MPI, recv_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    memcpy(extended_input + STENCIL_WIDTH / 2, input_MPI, recv_count * sizeof(double));

    // Start timer
    double local_start_time, local_elapsed_time, max_elapsed_time;
    MPI_Barrier(MPI_COMM_WORLD); 
    local_start_time = MPI_Wtime();

    // Main loop for the stencil application
    for (int s = 0; s < num_steps; s++) {
        exchange_ghost_cells_non_blocking(id, procs, EXTENT, recv_count, input_MPI, extended_input, request);

        // Apply stencil
        for (int i = 0; i < recv_count; i++) {
            double result = 0;
            for (int j = -EXTENT; j <= EXTENT; j++) {
                int index = i + EXTENT + j;
                result += STENCIL[j + EXTENT] * extended_input[index];
            }
            output_MPI[i] = result;
        }

        MPI_Waitall(4, request, MPI_STATUSES_IGNORE);

        // Swap input and output
        if (s < num_steps - 1) {
            double *tmp = input_MPI;
            input_MPI = output_MPI;
            output_MPI = tmp;
            memcpy(extended_input + EXTENT, input_MPI, recv_count * sizeof(double));
        }
    }

    // Stop timer
    local_elapsed_time = MPI_Wtime() - local_start_time;
    MPI_Reduce(&local_elapsed_time, &max_elapsed_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Gather the processed data from all processes to the root process
    MPI_Gather(output_MPI, recv_count, MPI_DOUBLE, output, recv_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Root process writes the result to the output file and prints the execution time
    if (id == 0) {
        printf("Maximum execution time across all processes: %f seconds\n", max_elapsed_time);
        if (0 != write_output(output_name, output, num_values)) {
            fprintf(stderr, "Failed to write output\n");
        }
        free(input);
        free(output);
    }

    free(input_MPI);
    free(output_MPI);
    free(extended_input);

    MPI_Finalize();

    return 0;
}

void exchange_ghost_cells_non_blocking(int id, int procs, int extent, int recv_count, double* local_data, double* extended_data, MPI_Request *request) {
    MPI_Status status;
    int left_rank = (id == 0) ? procs - 1 : id - 1;
    int right_rank = (id == procs - 1) ? 0 : id + 1;

    // Send to the right, receive from the left
    MPI_Isend(&local_data[recv_count - extent], extent, MPI_DOUBLE, right_rank, 0, MPI_COMM_WORLD, &request[0]);
    MPI_Irecv(extended_data, extent, MPI_DOUBLE, left_rank, 0, MPI_COMM_WORLD, &request[1]);

    // Send to the left, receive from the right
    MPI_Isend(local_data, extent, MPI_DOUBLE, left_rank, 1, MPI_COMM_WORLD, &request[2]);
    MPI_Irecv(&extended_data[recv_count + extent], extent, MPI_DOUBLE, right_rank, 1, MPI_COMM_WORLD, &request[3]);
}

int read_input(const char *file_name, double **values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "r"))) {
		perror("Couldn't open input file");
		return -1;
	}
	int num_values;
	if (EOF == fscanf(file, "%d", &num_values)) {
		perror("Couldn't read element count from input file");
		return -1;
	}
	if (NULL == (*values = malloc(num_values * sizeof(double)))) {
		perror("Couldn't allocate memory for input");
		return -1;
	}
	for (int i=0; i<num_values; i++) {
		if (EOF == fscanf(file, "%lf", &((*values)[i]))) {
			perror("Couldn't read elements from input file");
			return -1;
		}
	}
	if (0 != fclose(file)){
		perror("Warning: couldn't close input file");
	}
	return num_values;
}


int write_output(char *file_name, const double *output, int num_values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "w"))) {
		perror("Couldn't open output file");
		return -1;
	}
	for (int i = 0; i < num_values; i++) {
		if (0 > fprintf(file, "%.4f ", output[i])) {
			perror("Couldn't write to output file");
		}
	}
	if (0 > fprintf(file, "\n")) {
		perror("Couldn't write to output file");
	}
	if (0 != fclose(file)) {
		perror("Warning: couldn't close output file");
	}
	return 0;
}