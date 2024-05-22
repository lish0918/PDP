/* ==========================================================================================
    File : shearsort_v2.c
    Situation: Exclude the case when the matrix size is not divisible by the number of processes
    Result: Weak Improvement
    Processes number 20, speedup around 7 times
===========================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int compare_asc(const void *a, const void *b);
int compare_desc(const void *a, const void *b);
int check_sorted(int *matrix, int n);

int main(int argc, char *argv[]) {
    int rank, size, n;
    int local_rows;
    int *matrix = NULL;
    int *local_data;
    double start_time, elapsed_time, max_time;
    char *input_filename, *output_filename;
    FILE *input_file, *output_file;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4) {
        if (rank == 0) {
            printf("Usage: %s <matrix_size> <input_file> <output_file>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    n = atoi(argv[1]);
    input_filename = argv[2];
    output_filename = argv[3];

    /* ============================================
       * Calculate local_rows for each rank
       How to make the transposition work if the matrix 
       is not divisible by the number of processes?
    ============================================ */
    local_rows = n / size;
    local_data = (int*)malloc(local_rows * n * sizeof(int));

    if (rank == 0) {
        if (n % size != 0) {
            printf("Error: Matrix size must be divisible by the number of processes\n");
            MPI_Finalize();
            return 1;
        }
    }


    if (rank == 0) {
        matrix = (int*)malloc(n * n * sizeof(int));
        input_file = fopen(input_filename, "r");
        if (input_file == NULL) {
            printf("Error: Unable to open input file %s\n", input_filename);
            MPI_Finalize();
            return 1;
        }
        for (int i = 0; i < n * n; i++) {
            fscanf(input_file, "%d", &matrix[i]);
        }
        fclose(input_file);
    }

    start_time = MPI_Wtime();

    MPI_Scatter(matrix, local_rows * n, MPI_INT, local_data, local_rows * n, MPI_INT, 0, MPI_COMM_WORLD);

    int d = ceil(log2(n));

    // Determine sorting order for each rank
    int should_reverse = 0; // Initial should_reverse for rank 0
    for (int i = 0; i < rank; i++) {
        if (local_rows % 2 != 0) {
            should_reverse = !should_reverse; // Toggle should_reverse if previous rank had odd rows
        }
    }

    for (int l = 1; l <= d + 1; l++) {
        // Row-wise sorting
        for (int i = 0; i < local_rows; i++) {
            if (!should_reverse) {
                // Reversed order: odd rows descending, even rows ascending
                if ((i % 2) == 0) {
                    qsort(&local_data[i * n], n, sizeof(int), compare_asc);
                } else {
                    qsort(&local_data[i * n], n, sizeof(int), compare_desc);
                }
            } else {
                // Regular order: odd rows ascending, even rows descending
                if ((i % 2) == 0) {
                    qsort(&local_data[i * n], n, sizeof(int), compare_desc);
                } else {
                    qsort(&local_data[i * n], n, sizeof(int), compare_asc);
                }
            }
        }

        if (l <= d) {           
            int *temp = (int*)malloc(local_rows * n * sizeof(int));

            /* ======================================================================
                * 1. Transpose the local data block
                Each process performs an internal transpose on 'size' blocks of size 
                'local_rows * local_rows' from left to right and stores them in temp.
            ====================================================================== */
            for (int i = 0; i < local_rows; i++) {
                for (int j = 0; j < size; j++) {
                    for (int k = j * local_rows; k < (j + 1) * local_rows; k++) {
                        temp[(k % local_rows) * n + i + j * local_rows] = local_data[i * n + k];
                    }
                }
            }

            /* ======================================================================
                * 2. Change to Alltoall Order
                Flatten the contents of 'size' blocks of size 'local_rows * local_rows' 
                from left to right into the entire array for 'Alltoall' communication.
            ====================================================================== */
            for (int j = 0; j < size; j++) {
                for (int i = 0; i < local_rows; i++) {
                    for (int k = j * local_rows; k < (j + 1) * local_rows; k++) {
                        local_data[(i + j * local_rows) * local_rows + k % local_rows] = temp[i * n + k];
                    }
                }
            }
            
            /* ======================================================================
                * 3. Alltoall Communication
                Merge the contents of the i-th block of size 'local_rows * local_rows' 
                from each process's local_data and transmit it to the i-th process's temp.
            ====================================================================== */
            MPI_Alltoall(local_data, local_rows * local_rows, MPI_INT, temp, local_rows * local_rows, MPI_INT, MPI_COMM_WORLD);
            
             /* ======================================================================
                * 4. Change Back to Normal Order
                The reverse operation of the second step, redistributing the flattened 
                content back into blocks.
            ====================================================================== */
            for (int j = 0; j < size; j++) {
                for (int i = 0; i < local_rows; i++) {
                    for (int k = j * local_rows; k < (j + 1) * local_rows; k++) {
                        local_data[i * n + k] = temp[(i + j * local_rows) * local_rows + k % local_rows];
                    }
                }
            }

            // Column-wise sorting
            for (int i = 0; i < local_rows; i++) {
                qsort(&local_data[i * n], n, sizeof(int), compare_asc);
            }

            // Block transposition
            for (int i = 0; i < local_rows; i++) {
                for (int j = 0; j < size; j++) {
                    for (int k = j * local_rows; k < (j + 1) * local_rows; k++) {
                        temp[(k % local_rows) * n + i + j * local_rows] = local_data[i * n + k];
                    }
                }
            }

            // Change to Alltoall Order
            for (int j = 0; j < size; j++) {
                for (int i = 0; i < local_rows; i++) {
                    for (int k = j * local_rows; k < (j + 1) * local_rows; k++) {
                        local_data[(i + j * local_rows) * local_rows + k % local_rows] = temp[i * n + k];
                    }
                }
            }

            MPI_Alltoall(local_data, local_rows * local_rows, MPI_INT, temp, local_rows * local_rows, MPI_INT, MPI_COMM_WORLD);
            
            // Change Back to Normal Order
            for (int j = 0; j < size; j++) {
                for (int i = 0; i < local_rows; i++) {
                    for (int k = j * local_rows; k < (j + 1) * local_rows; k++) {
                        local_data[i * n + k] = temp[(i + j * local_rows) * local_rows + k % local_rows];
                    }
                }
            }

            free(temp);
        }
    }

    MPI_Gather(local_data, local_rows * n, MPI_INT, matrix, local_rows * n, MPI_INT, 0, MPI_COMM_WORLD);

    elapsed_time = MPI_Wtime() - start_time;
    MPI_Reduce(&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        output_file = fopen(output_filename, "w");
        if (output_file == NULL) {
            printf("Error: Unable to open output file %s\n", output_filename);
            MPI_Finalize();
            return 1;
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                fprintf(output_file, "%d ", matrix[i * n + j]);
            }
            fprintf(output_file, "\n");
        }
        fclose(output_file);

        if (check_sorted(matrix, n)) {
            printf("The matrix is correctly sorted.\n");
        } else {
            printf("The matrix is NOT correctly sorted.\n");
        }

        printf("Execution Time: %f seconds\n", max_time);
        free(matrix);
    }

    free(local_data);
    MPI_Finalize();
    return 0;
}

int compare_asc(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int compare_desc(const void *a, const void *b) {
    return (*(int*)b - *(int*)a);
}

int check_sorted(int *matrix, int n) {
    // Check the matrix is snake-size sorted
    int prev = matrix[0];
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < n; j++) {
                if (matrix[i * n + j] < prev) {
                    return 0;
                }
                prev = matrix[i * n + j];
            }
        } else {
            for (int j = n - 1; j >= 0; j--) {
                if (matrix[i * n + j] < prev) {
                    return 0;
                }
                prev = matrix[i * n + j];
            }
        }
    }

    return 1;
}
