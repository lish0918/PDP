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
    int *rows_per_rank = (int*)malloc(size * sizeof(int));

    // Calculate local_rows for each rank
    int base_rows = n / size;
    int remainder = n % size;
    for (int i = 0; i < size; i++) {
        rows_per_rank[i] = base_rows + (i < remainder ? 1 : 0);
        sendcounts[i] = rows_per_rank[i] * n;
        displs[i] = (i == 0) ? 0 : displs[i-1] + sendcounts[i-1];
    }
    //printf("sendcounts:%d, displs:%d, rank:%d\n", sendcounts[rank], displs[rank], rank);

    local_rows = rows_per_rank[rank];
    local_data = (int*)malloc(local_rows * n * sizeof(int));

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
    //printf("Rank %d should_reverse: %d\n", rank, should_reverse);

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

        if(l <= d){
            MPI_Gatherv(local_data, local_rows * n, MPI_INT, matrix, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

            int *temp = (int*)malloc(n * n * sizeof(int));
            if (rank == 0) {
                // Transpose matrix
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        temp[i * n + j] = matrix[j * n + i];
                    }
                }

                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        matrix[i * n + j] = temp[i * n + j];
                    }
                }
            }

            MPI_Scatterv(matrix, sendcounts, displs, MPI_INT, local_data, local_rows * n, MPI_INT, 0, MPI_COMM_WORLD);

            // Column-wise sorting
            for (int i = 0; i < local_rows; i++) {
                qsort(&local_data[i * n], n, sizeof(int), compare_asc);
            }            

            MPI_Gatherv(local_data, local_rows * n, MPI_INT, matrix, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

            if (rank == 0) {   
                // Transpose matrix back
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        temp[i * n + j] = matrix[j * n + i];
                    }
                }

                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        matrix[i * n + j] = temp[i * n + j];
                    }
                }
            }

            MPI_Scatterv(matrix, sendcounts, displs, MPI_INT, local_data, local_rows * n, MPI_INT, 0, MPI_COMM_WORLD);

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
