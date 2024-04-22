#include "stencil.h"

int main(int argc, char** argv) {
    /* Initialize MPI */
    int myid, numprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Status stat;

    if (4 != argc) {
        printf("Usage: stencil input_file output_file number_of_applications\n");
        return 1;
    }
    char* input_name = argv[1];
    char* output_name = argv[2];
    int num_steps = atoi(argv[3]);

	/* For time recording */
    double start;

    
    /* Read input file: Process 0 */
    int num_values;
	double* input; /* Requred be freed */
    if (myid == 0) {
        if (0 > (num_values = read_input(input_name, &input))) {
            return 2;
        }
		/* Start timer */
		start = MPI_Wtime();
    }

	/* Broadcast number of values to all processes */
    MPI_Bcast(&num_values, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* Stencil values */
    double h = 2.0 * PI / num_values;
    const int STENCIL_WIDTH = 5;
    const int EXTENT = STENCIL_WIDTH / 2;
    const double STENCIL[] = {1.0 / (12 * h), -8.0 / (12 * h), 0.0, 8.0 / (12 * h), -1.0 / (12 * h)};
    // const double STENCIL[] = {1.0 / 12, -8.0 / 12, 0.0, 8.0 / 12, -1.0 / 12};

    /**
     * Idea:
     * - distribute the data to the processes evenly
     * - each process will have a local copy of the data
     * - assume the local data for each process is local_N, then the will be two level of halo points, so the size of the local data should be local_N + 2 * 2 （EXTENT = 2)
     * - excahnge the data between the processes frist, then do the computation
     * 	- the data exchange is done by using MPI_Send and MPI_Recv, the data shoul be exchanged are the halo points, which are the first two and the last two points of the local data
     */

    /* Allocate data evenly for local input */
    int group_size = num_values / numprocs;
    if (myid < num_values % numprocs) {
        group_size += 1;
    }
    double* local_input = malloc((group_size + 2 * EXTENT) * sizeof(double));

    /* Distrubute the data to the processes from Process 0 */
    if (myid == 0) {
        /* Local data for Process 0 */
        for (int i = 0; i < group_size; i++) {
            local_input[i + EXTENT] = input[i];
        }
        /* Send data to other processes */
        for (int i = 1; i < numprocs; i++) {
			/* For Process 0 to send data */
            int local_group_size = num_values / numprocs;
            if (i < num_values % numprocs) {
                local_group_size += 1;
            }
            MPI_Send(input + i * local_group_size, local_group_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
    } else {
		/* local_input[EXTENT] */
        MPI_Recv(local_input + EXTENT, group_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    /* Allocate data for local results */
    double* local_output;
    if (NULL == (local_output = malloc((group_size + 2 * EXTENT) * sizeof(double)))) {
        perror("Couldn't allocate memory for output");
        return 2;
    }

	/* COMPUTATION */
	/* Elements => local_input: EXTENT GROUP_SIZE EXTENT */
    for (int s = 0; s < num_steps; s++) {
        /* Initialize the halo points */
		int source, destination;

		destination = myid == 0 ? numprocs - 1 : myid - 1;
		source = myid == numprocs - 1 ? 0 : myid + 1;
		MPI_Send(local_input + EXTENT, EXTENT, MPI_DOUBLE, destination, 0, MPI_COMM_WORLD);
		MPI_Recv(local_input + EXTENT + group_size, EXTENT, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


		destination = myid == numprocs - 1 ? 0 : myid + 1;
		source = myid == 0 ? numprocs - 1 : myid - 1;
		MPI_Send(local_input + group_size, EXTENT, MPI_DOUBLE, destination, 0, MPI_COMM_WORLD);
    	MPI_Recv(local_input, EXTENT, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // // print local data
        // for (int i = 0; i < group_size + 2 * EXTENT; i++) {
        //     printf("Process %d: local_input[%d] = %f\n", myid, i, local_input[i]);
        // }

        /* Local computation */
        for (int i = EXTENT; i < group_size + EXTENT; i++) {
            double result = 0;
            for (int j = 0; j < STENCIL_WIDTH; j++) {
                int index = i - EXTENT + j;
                result += STENCIL[j] * local_input[index];
            }
            local_output[i] = result;
        }

        // // print local data
        // for (int i = 0; i < group_size + 2 * EXTENT; i++) {
        //     printf("Process %d: local_output[%d] = %f\n", myid, i, local_output[i]);
        // }

		double* temp = local_input;
        local_input = local_output;
		local_output = temp;
    }
	free(local_output);

    /* Gather the data from all the processes */
    if (myid == 0) {
        for (int i = 0; i < group_size; i++) {
            input[i] = local_input[i + EXTENT];
        }
        for (int i = 1; i < numprocs; i++) {
            int local_group_size = num_values / numprocs;
            if (i < num_values % numprocs) {
                local_group_size += 1;
            }
            MPI_Recv(input + i * local_group_size, local_group_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(local_input + EXTENT, group_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

	/* Write result */
	if (myid == 0)
	{
		/* Stop timer */
		printf("%f\n", MPI_Wtime() - start);
		#ifdef PRODUCE_OUTPUT_FILE
			if (0 != write_output(output_name, input, num_values)) {
				return 2;
			}
		#endif
		// Clean up
		free(input);
	}
	free(local_input);
	MPI_Finalize();
    return 0;
}

int read_input(const char* file_name, double** values) {
    FILE* file;
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
    for (int i = 0; i < num_values; i++) {
        if (EOF == fscanf(file, "%lf", &((*values)[i]))) {
            perror("Couldn't read elements from input file");
            return -1;
        }
    }
    if (0 != fclose(file)) {
        perror("Warning: couldn't close input file");
    }
    return num_values;
}

int write_output(char* file_name, const double* output, int num_values) {
    FILE* file;
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
