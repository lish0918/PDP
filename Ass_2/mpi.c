#include "stencil.h"


int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Check arguments
	if (argc != 4) {
        if(rank == 0){
            printf("Usage: stencil input_file output_file number_of_applications\n");
        }
        MPI_Finalize();
		return 1;
	}

	char *input_name = argv[1];
	char *output_name = argv[2];
	int num_steps = atoi(argv[3]);

	// Read input file
	double *input;
    int local_num_values;

    if(rank == 0){
        int num_values;
        if (0 > (num_values = read_input(input_name, &input))) {
            MPI_Finalize();
            return 2;
        }
        local_num_values = num_values / size;
    }
	
    MPI_Bcast(&num_values, 1, MPI_INT, 0, MPI_COMM_WORLD);

    double *local_input = (double *)malloc(local_num_values * sizeof(double));
    double *local_output = (double *)malloc(local_num_values * sizeof(double));

    MPI_Scatter(input, local_num_values, MPI_DOUBLE, local_input, local_num_values, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    
    // Calculate stencil values
    double h = 2.0*PI/num_values;
    const int STENCIL_WIDTH = 5;
    const int EXTENT = STENCIL_WIDTH/2;
    const double STENCIL[] = {1.0/(12*h), -8.0/(12*h), 0.0, 8.0/(12*h), -1.0/(12*h)};

    // Start timer
    double start = MPI_Wtime();

	// Repeatedly apply stencil
	for (int s=0; s<num_steps; s++) {
		// Apply stencil
		for (int i=EXTENT; i<local_num_values -EXTENT; i++) {
			double result = 0;
			for (int j=0; j<STENCIL_WIDTH; j++) {
				int index = i - EXTENT + j;
				result += STENCIL[j] * local_input[index];
			}
			local_output[i] = result;
		}

		MPI_Request request[4];
		MPI_Isend(&local_output[EXTENT], 1, MPI_DOUBLE, (rank + 1) % size, 0, MPI_COMM_WORLD, &reqs[0]);
        MPI_Isend(&local_output[local_num_values - 2 * EXTENT - 1], 1, MPI_DOUBLE, (rank + size - 1) % size, 1, MPI_COMM_WORLD, &reqs[1]);
        MPI_Irecv(&local_output[0], 1, MPI_DOUBLE, (rank + size - 1) % size, 0, MPI_COMM_WORLD, &reqs[2]);
        MPI_Irecv(&local_output[local_num_values - EXTENT], 1, MPI_DOUBLE, (rank + 1) % size, 1, MPI_COMM_WORLD, &reqs[3]);

		MPI_Waitall(4, request, MPI_STATUS_IGNORE);

		// Swap input and output
		double *tmp = input;
		input = output;
		output = tmp;
	}
	free(input);

	// Stop timer
	double my_execution_time = MPI_Wtime() - start;

	// Gather results
	MPI_Gather(local_output, local_num_values, MPI_DOUBLE, output, local_num_values, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	if(rank == 0){
		// Write result
		printf("%f\n", my_execution_time);
		if (0 != write_output(output_name, output, num_values)) {
			MPI_Finalize();
			return 2;
		}
	}

	// Clean up
	free(output);
	free(local_input);
	free(local_output);

	MPI_Finalize();
	return 0;
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
