/* ==========================================================================================
    File : shearsort_v3.c
    Situation: Include the case when the matrix size is not divisible by the number of processes
    Result: 
    
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

    int *sendcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    int *rcounts = (int*)malloc(size * sizeof(int));
    int *rdispls = (int*)malloc(size * sizeof(int));
    int *rows_per_rank = (int*)malloc(size * sizeof(int));

    int base_rows = n / size;
    int remainder = n % size;

    // Calculate sendcounts and displs for Scatterv
    for (int i = 0; i < size; i++) {
        rows_per_rank[i] = base_rows + (i < remainder ? 1 : 0);
        sendcounts[i] = rows_per_rank[i] * n;
        displs[i] = (i == 0) ? 0 : displs[i-1] + sendcounts[i-1];
    }
    
    // Allocate memory for local data
    local_rows = rows_per_rank[rank];
    local_data = (int*)malloc(local_rows * n * sizeof(int));

    // Calculate rcounts and rdispls for Alltoallv
    for (int i = 0; i < size; i++) {
        rcounts[i] = rows_per_rank[i] * local_rows;
        rdispls[i] = (i == 0) ? 0 : rdispls[i-1] + rcounts[i-1];
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

    MPI_Scatterv(matrix, sendcounts, displs, MPI_INT, local_data, local_rows * n, MPI_INT, 0, MPI_COMM_WORLD);

    int d = ceil(log2(n));

    // Determine sorting order for each rank
    int should_reverse = 0; // Initial should_reverse for rank 0
    for (int i = 0; i < rank; i++) {
        if (rows_per_rank[i] % 2 != 0) {
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
                * 1. Reorder the local data for Alltoallv
                Read data from the first column of the two-dimensional array represented 
                by the one-dimensional array 'local_data' to the one-dimensional array 
                'temp' in left-to-right order.
            ====================================================================== */            
            int index = 0;
            for (int col = 0; col < n; col++) {
                for (int row = 0; row < local_rows; row++) {
                    temp[index++] = local_data[row * n + col];
                }
            }    

            /* ======================================================================
                * 2. Alltoallv Communication
                Merge the contents of the i-th block from each process's 'temp' and 
                transmit it to the i-th process's 'local_data'.
            ====================================================================== */            
            MPI_Alltoallv(temp, rcounts, rdispls, MPI_INT, local_data, rcounts, rdispls, MPI_INT, MPI_COMM_WORLD);                    
            
             /* ======================================================================
                * 3. Reorder again to achieve transposition
                The reverse operation of the second step, redistributing the flattened 
                content back into blocks.
            ====================================================================== */
            index = 0;
            for (int i = 0; i < local_rows; i++) {
                int max = 0;
                int t = 0;
                for (int j = 0; j < size; j++) {
                    if (j != 0) {
                        t = rows_per_rank[j - 1];
                    }
                    max = max + t * local_rows;
                    for (int k = max + i * rows_per_rank[j]; k < max + (i + 1) * rows_per_rank[j]; k++) {
                        temp[index++] = local_data[k];
                    }
                }
            } 

            // Column-wise sorting
            for (int i = 0; i < local_rows; i++) {
                qsort(&temp[i * n], n, sizeof(int), compare_asc);
            }            

            // Reorder the local data for Alltoallv
            index = 0;
            for (int col = 0; col < n; col++) {
                for (int row = 0; row < local_rows; row++) {
                    local_data[index++] = temp[row * n + col];
                }
            }   

            // Alltoallv Communication          
            MPI_Alltoallv(local_data, rcounts, rdispls, MPI_INT, temp, rcounts, rdispls, MPI_INT, MPI_COMM_WORLD);                    
            
             // Reorder again to achieve transposition
            index = 0;
            for (int i = 0; i < local_rows; i++) {
                int max = 0;
                int t = 0;
                for (int j = 0; j < size; j++) {
                    if (j != 0) {
                        t = rows_per_rank[j - 1];
                    }
                    max = max + t * local_rows;
                    for (int k = max + i * rows_per_rank[j]; k < max + (i + 1) * rows_per_rank[j]; k++) {
                        local_data[index++] = temp[k];
                    }
                }
            }

            free(temp);
        }
    }

    MPI_Gatherv(local_data, local_rows * n, MPI_INT, matrix, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

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
    free(sendcounts);
    free(displs);
    free(rcounts);
    free(rdispls);
    free(rows_per_rank);
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
