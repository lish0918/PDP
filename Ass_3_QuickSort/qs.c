#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

// Helper function to compare integers (used by qsort)
int compare_integers(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Function to perform local quicksort on a segment of the array
void local_quicksort(int *array, int start, int end) {
    qsort(&array[start], end - start + 1, sizeof(int), compare_integers);
}

// Function to select the pivot based on the chosen strategy
int select_pivot(int *array, int start, int end, int strategy, MPI_Comm comm) {
    int local_size = end - start + 1;
    int *local_medians = (int *)malloc(local_size * sizeof(int));

    // Collect local medians
    for (int i = start; i <= end; i++) {
        local_medians[i - start] = array[i];
    }

    // Sort local medians
    local_quicksort(local_medians, 0, local_size - 1);

    int global_median = 0;

    if (strategy == 1) {
        // Strategy 1: Randomly select one local median as global pivot
        int random_index = start + rand() % local_size;
        global_median = local_medians[random_index - start];
    } else if (strategy == 2) {
        // Strategy 2: Select median of all local medians as global pivot
        global_median = local_medians[local_size / 2];
    } else if (strategy == 3) {
        // Strategy 3: Select mean value of all local medians as global pivot
        int sum = 0;
        for (int i = 0; i < local_size; i++) {
            sum += local_medians[i];
        }
        global_median = sum / local_size;
    }

    free(local_medians);

    return global_median;
}

// Parallel quicksort implementation
void parallel_quicksort(int *array, int start, int end, int pivot_strategy, MPI_Comm comm) {
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    if (size == 1) {
        local_quicksort(array, start, end);
        return;
    }

    int pivot;
    if (rank == 0) {
        // Choose pivot based on the selected strategy
        pivot = select_pivot(array, start, end, pivot_strategy, comm);
    }

    // Broadcast pivot to all processes
    MPI_Bcast(&pivot, 1, MPI_INT, 0, comm);

    // Partition the array based on the pivot
    int i = start, j = end;
    while (i <= j) {
        while (array[i] < pivot) {
            i++;
        }
        while (array[j] > pivot) {
            j--;
        }
        if (i <= j) {
            // Swap elements
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
            i++;
            j--;
        }
    }

    // Create subgroups
    MPI_Comm new_comm;
    MPI_Comm_split(comm, (array[start] <= pivot), rank, &new_comm);

    int new_size, new_rank;
    MPI_Comm_size(new_comm, &new_size);
    MPI_Comm_rank(new_comm, &new_rank);

    // Recursively sort the subgroups
    if (new_rank != MPI_UNDEFINED) {
        int mid = i; // Midpoint after partition
        if (new_rank == 0) {
            parallel_quicksort(array, start, mid - 1, pivot_strategy, new_comm);
        } else {
            parallel_quicksort(array, mid, end, pivot_strategy, new_comm);
        }
    }

    MPI_Comm_free(&new_comm);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <pivot_strategy>\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char *input_file = argv[1];
    char *output_file = argv[2];
    int pivot_strategy = atoi(argv[3]);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if ((size & (size - 1)) != 0) {
        if (rank == 0) {
            fprintf(stderr, "Number of processes must be a power of two.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int n, *data = NULL;

    if (rank == 0) {
        FILE *fp = fopen(input_file, "r");
        if (fp == NULL) {
            fprintf(stderr, "Error: Unable to open input file.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fscanf(fp, "%d", &n);
        data = (int *)malloc(n * sizeof(int));

        for (int i = 0; i < n; i++) {
            fscanf(fp, "%d", &data[i]);
        }

        fclose(fp);
    }

    double start_time = MPI_Wtime();

    // Broadcast the number of elements to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (data == NULL) {
        data = (int *)malloc(n * sizeof(int));
    }

    // Scatter the data to all processes
    MPI_Scatter(data, n / size, MPI_INT, data, n / size, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform parallel quicksort
    parallel_quicksort(data, 0, n / size - 1, pivot_strategy, MPI_COMM_WORLD);

    // Gather sorted data
    MPI_Gather(data, n / size, MPI_INT, data, n / size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        FILE *fp = fopen(output_file, "w");
        if (fp == NULL) {
            fprintf(stderr, "Error: Unable to open output file.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        for (int i = 0; i < n; i++) {
            fprintf(fp, "%d ", data[i]);
        }
        fclose(fp);

        // Calculate sorting time
        double end_time = MPI_Wtime();
        printf("Sorting time: %.6f seconds\n", end_time - start_time);
    }

    free(data);
    MPI_Finalize();

    return 0;
}
