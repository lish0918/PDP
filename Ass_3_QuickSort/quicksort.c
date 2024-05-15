#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int calculate_pivot(int *chunk, int chunk_size, int id, int p, int pivot_strategy, MPI_Comm comm) {
    int median, final_pivot = 0;
    qsort(chunk, chunk_size, sizeof(int), compare);
    median = (chunk_size % 2 == 0) ? (chunk[chunk_size / 2 - 1] + chunk[chunk_size / 2]) / 2 : chunk[chunk_size / 2];

    if (pivot_strategy == 1) { 
        final_pivot = median;
    } else {
        int *medians = NULL;
        if (id == 0) medians = (int *)malloc(p * sizeof(int));
        MPI_Gather(&median, 1, MPI_INT, medians, 1, MPI_INT, 0, comm);

        if (id == 0) {
            if (pivot_strategy == 2) {  // Median of medians
                qsort(medians, p, sizeof(int), compare);
                final_pivot = medians[p / 2];
            } else if (pivot_strategy == 3) {  // Mean of medians
                int sum = 0;
                for (int i = 0; i < p; i++) sum += medians[i];
                final_pivot = sum / p;
            }
            free(medians);
        }
        MPI_Bcast(&final_pivot, 1, MPI_INT, 0, comm); 
    }
    return final_pivot;
}

int* merge(int *v1, int n1, int *v2, int n2) {
    int *result = (int*)malloc((n1 + n2) * sizeof(int));
    int i = 0, j = 0, k = 0;
    while (i < n1 && j < n2) {
        if (v1[i] < v2[j]) result[k++] = v1[i++];
        else result[k++] = v2[j++];
    }
    while (i < n1) result[k++] = v1[i++];
    while (j < n2) result[k++] = v2[j++];
    return result;
}

void exchange_chunks(int *chunk, int size, int low, int high, int pair, int tag, MPI_Comm comm, int *new_size, int **new_chunk) {
    MPI_Status status;
    MPI_Request request_send, request_recv;
    
    MPI_Isend(chunk + low, high, MPI_INT, pair, tag, comm, &request_send);
    MPI_Probe(pair, 1 - tag, comm, &status);
    MPI_Get_count(&status, MPI_INT, new_size);
    *new_chunk = (int *)malloc(*new_size * sizeof(int));
    MPI_Irecv(*new_chunk, *new_size, MPI_INT, pair, 1 - tag, comm, &request_recv);

    MPI_Wait(&request_send, &status);
    MPI_Wait(&request_recv, &status);
}

int main(int argc, char** argv) {
    int id, p, n, *data, *chunk, *temp, *other;
    MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (argc != 4) {
        if (id == 0) fprintf(stderr, "Usage: %s <input_file> <output_file> <pivot_strategy>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int pivot_strategy = atoi(argv[3]);

    if (id == 0) {
        FILE *fp = fopen(argv[1], "r");
        if (!fp) {
            fprintf(stderr, "Failed to open file %s\n", argv[1]);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            return 1;  // Adding return here to indicate failure clearly
        }
        fscanf(fp, "%d", &n);
        data = (int *)malloc(n * sizeof(int));
        if (!data) {
            fprintf(stderr, "Memory allocation failed\n");
            fclose(fp);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            return 1;  // Ensure to exit after freeing resources
        }
        for (int i = 0; i < n; i++) {
            fscanf(fp, "%d", &data[i]);
        }
        fclose(fp);
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int chunk_size = n / p;
    int remainder = n % p;
    int *send_counts = (int *)malloc(p * sizeof(int));
    int *displacements = (int *)malloc(p * sizeof(int));

    for (int i = 0; i < p; i++) {
        send_counts[i] = (i < remainder) ? (chunk_size + 1) : chunk_size;
        displacements[i] = (i > 0) ? (displacements[i - 1] + send_counts[i - 1]) : 0;
    }

    chunk = (int *)malloc(send_counts[id] * sizeof(int));
    //printf("id: %d, chunk_size: %d\n", id, send_counts[id]);
    MPI_Scatterv(data, send_counts, displacements, MPI_INT, chunk, send_counts[id], MPI_INT, 0, MPI_COMM_WORLD);
    //printf("After Scatter id: %d, chunk_size: %d\n", id, chunk_size);

    double start_time = MPI_Wtime();
    double max_time = 0.0;

    if (p = 1){
        qsort(chunk, chunk_size, sizeof(int), compare);
        max_time = MPI_Wtime() - start_time;
    }
    else{
        int group_size = p;
        int group_id = id;
        int pivot;
        MPI_Comm comm = MPI_COMM_WORLD;

        while (group_size > 1) {
            pivot = calculate_pivot(chunk, chunk_size, group_id, group_size, pivot_strategy, comm);

            int pivotIndex = chunk_size / 2;
            while (pivotIndex > 0 && chunk[pivotIndex] >= pivot) {
                pivotIndex--;
            }
            int low = pivotIndex;
            int high = chunk_size - pivotIndex;

            int pair;
            if (group_id < group_size / 2) {
                pair = group_id + group_size / 2;
            } else {
                pair = group_id - group_size / 2;
            }

            int new_size;
            int *new_chunk = NULL;
            
            if (group_id < group_size / 2) {
                exchange_chunks(chunk, chunk_size, low, high, pair, 0, comm, &new_size, &new_chunk);
            } else {
                exchange_chunks(chunk, chunk_size, 0, low, pair, 1, comm, &new_size, &new_chunk);
            }

            if (group_id < group_size / 2) {
                chunk_size = low + new_size;
                temp = merge(chunk, low, new_chunk, new_size);
            } else {
                chunk_size = high + new_size;
                temp = merge(chunk + low, high, new_chunk, new_size);
            }

            free(chunk);
            chunk = temp;

            MPI_Comm newcomm;
            MPI_Comm_split(comm, group_id < group_size / 2, group_id, &newcomm);
            MPI_Comm_rank(newcomm, &group_id);
            MPI_Comm_size(newcomm, &group_size);
            comm = newcomm;

            free(new_chunk);
        }

        double end_time = MPI_Wtime();
        double elapsed_time = end_time - start_time;
        // The maximum time taken by any process
        MPI_Allreduce(&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
        
        // Tree-based merge
        int step = 1;
        while (step < p) {
            if (id % (2 * step) == 0) {
                int sender = id + step;
                if (sender < p) {
                    int new_size;
                    MPI_Recv(&new_size, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, &status);
                    other = (int *)malloc(new_size * sizeof(int));
                    MPI_Recv(other, new_size, MPI_INT, sender, 0, MPI_COMM_WORLD, &status);
                    temp = merge(chunk, chunk_size, other, new_size);
                    free(chunk);
                    free(other);
                    chunk = temp;
                    chunk_size += new_size;
                }
            } else {
                int receiver = id - step;
                MPI_Send(&chunk_size, 1, MPI_INT, receiver, 0, MPI_COMM_WORLD);
                MPI_Send(chunk, chunk_size, MPI_INT, receiver, 0, MPI_COMM_WORLD);
                break;
            }
            step *= 2;
        }
    }

    if (id == 0) {
        printf("Elapsed time: %f\n", max_time);

        FILE *fo = fopen(argv[2], "w");
        if (!fo) {
            fprintf(stderr, "Failed to open file %s\n", argv[2]);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        for (int i = 0; i < chunk_size; i++) {
            fprintf(fo, "%d ", chunk[i]);
        }
        fprintf(fo, "\n");
        fclose(fo);
    }

    free(chunk);
    if (id == 0) {
        free(data);
    }

    MPI_Finalize();

    return 0;
}
