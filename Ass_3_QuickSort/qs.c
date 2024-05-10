#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>

#define MAX_FILENAME_LENGTH 256

// Function to swap two integers
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Partition function to divide data based on pivot
int partition(int *arr, int low, int high, int pivot) {
    int i = low - 1;
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Parallel quicksort function using MPI
void parallel_quicksort(int *arr, int n, int my_rank, int p, MPI_Comm comm, int pivot_strategy) {
    int *local_data;
    int local_n;
    int *all_pivots;
    int *pivots;
    int pivot;

    /* 进程0把需要排序的数组均分给p个进程，
    每个进程内进行局部排序，然后选取p个中位数作为每个进程的pivot，
    将这些pivot发送给进程0，进程0随机选取一个pivot作为全局pivot（策略1），
    进程0选取这些pivot的中位数作为全局pivot（策略2），
    进程0选取所有pivot的均值数作为全局pivot（策略3），
    然后广播给所有进程，每个进程根据全局pivot将数据分为两部分，
    一部分小于pivot，一部分大于pivot，然后将这两部分数据发送给不同的进程，
    递归的进行这个过程，直到数据有序为止。
    */

    // Scatter data to all processes
    local_n = n / p;
    local_data = (int *)malloc(local_n * sizeof(int));
    MPI_Scatter(arr, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, comm);

    // Local sort
    qsort(local_data, local_n, sizeof(int), compare);

    // Gather all pivots to process 0
    all_pivots = (int *)malloc(p * sizeof(int));
    pivots = (int *)malloc((p - 1) * sizeof(int));
    MPI_Gather(&local_data[local_n / 2], 1, MPI_INT, all_pivots, 1, MPI_INT, 0, comm);

    // Select pivot based on strategy
    if (my_rank == 0) {
        qsort(all_pivots, p, sizeof(int), compare);
        switch (pivot_strategy) {
            case 1:
                pivot = all_pivots[rand() % p]; // Random pivot strategy
                break;
            case 2:
                pivot = all_pivots[p / 2]; // Median of medians pivot strategy
                break;
            case 3:{
                int sum = 0;
                for (int i = 0; i < p; i++) {
                    sum += all_pivots[i];
                }
                pivot = sum / p;
                break;
            }
            default:
                pivot = all_pivots[p / 2]; // Default to median of medians
                break;
        }
    }
    MPI_Bcast(&pivot, 1, MPI_INT, 0, comm);

    // Partition local data based on pivot
    int i = 0, j = local_n - 1;
    while (i <= j) {
        while (local_data[i] < pivot)
            i++;
        while (local_data[j] > pivot)
            j--;
        if (i <= j) {
            swap(&local_data[i], &local_data[j]);
            i++;
            j--;
        }
    }

    // Calculate split point for data exchange
    int split = i;

    // Exchange data between processes based on pivot
    int color = (my_rank >= split) ? 1 : 0;
    MPI_Comm split_comm;
    MPI_Comm_split(comm, color, my_rank, &split_comm);

    // Recursively sort subgroups
    if (my_rank < split) {
        parallel_quicksort(local_data, split, my_rank, p, split_comm, pivot_strategy);
    } else {
        parallel_quicksort(&local_data[split], local_n - split, my_rank - split, p, split_comm, pivot_strategy);
    }

    // Gather sorted data back to process 0
    MPI_Gather(local_data, local_n, MPI_INT, arr, local_n, MPI_INT, 0, comm);

    // Clean up
    free(local_data);
    free(all_pivots);
    free(pivots);
}

int main(int argc, char *argv[]) {
    int my_rank, p;
    int *data;
    int n;
    double start_time, end_time;
    int pivot_strategy;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (argc < 4) {
        if (my_rank == 0) {
            printf("Usage: %s <input_file> <output_file> <pivot_strategy>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    char input_filename[MAX_FILENAME_LENGTH];
    char output_filename[MAX_FILENAME_LENGTH];
    strcpy(input_filename, argv[1]);
    strcpy(output_filename, argv[2]);
    pivot_strategy = atoi(argv[3]);

    // Process 0 reads input data from file
    if (my_rank == 0) {
        FILE *file = fopen(input_filename, "r");
        if (!file) {
            printf("Error: Unable to open input file.\n");
            MPI_Finalize();
            return 1;
        }
        fscanf(file, "%d", &n);
        data = (int *)malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            fscanf(file, "%d", &data[i]);
        }
        fclose(file);
    }

    // Broadcast the number of elements to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for local data
    int local_n = n / p;
    int *local_data = (int *)malloc(local_n * sizeof(int));

    // Scatter data to all processes
    MPI_Scatter(data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Measure sorting time (excluding file I/O)
    start_time = MPI_Wtime();

    // Perform parallel quicksort
    parallel_quicksort(local_data, local_n, my_rank, p, MPI_COMM_WORLD, pivot_strategy);

    end_time = MPI_Wtime();

    // Gather sorted data back to process 0
    MPI_Gather(local_data, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Output sorted data to file by process 0
    if (my_rank == 0) {
        FILE *file = fopen(output_filename, "w");
        if (!file) {
            printf("Error: Unable to open output file.\n");
            MPI_Finalize();
            return 1;
        }
        fprintf(file, "%d\n", n);
        for (int i = 0; i < n; i++) {
            fprintf(file, "%d ", data[i]);
        }
        fclose(file);

        // Print sorting time
        printf("%.6f\n", end_time - start_time);
    }

    // Clean up
    free(data);
    free(local_data);
    MPI_Finalize();

    return 0;
}
