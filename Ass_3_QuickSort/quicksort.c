#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int partition(int arr[], int low, int high, int pivot) {
    int i = low - 1;
    for (int j = low; j <= high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    int pivotIndex = i + 1;
    for (int j = pivotIndex; j <= high; j++) {
        if (arr[j] == pivot) {
            swap(&arr[pivotIndex], &arr[j]);
            break;
        }
    }
    return pivotIndex;
}

int calculate_pivot(int *chunk, int chunk_size, int id, int p, int pivot_strategy, MPI_Comm comm) {
    int median, final_pivot = 0;
    qsort(chunk, chunk_size, sizeof(int), compare);
    median = (chunk_size % 2 == 0) ? (chunk[chunk_size / 2 - 1] + chunk[chunk_size / 2]) / 2 : chunk[chunk_size / 2];

    if (pivot_strategy == 1) {
        // Strategy 1: Local median as pivot (if used directly in partitioning)
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

void quicksort(int arr[], int low, int high, int id, int p, int pivot_strategy, MPI_Comm comm) {
    if (low < high) {
        int pivot = calculate_pivot(arr + low, high - low + 1, id, p, pivot_strategy, comm);
        int pi = partition(arr, low, high, pivot);
        quicksort(arr, low, pi - 1, id, p, pivot_strategy, comm);
        quicksort(arr, pi + 1, high, id, p, pivot_strategy, comm);
    }
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

int main(int argc, char** argv) {
    int id, p, n, *data, *chunk, *temp, *other;
    MPI_Init(&argc, &argv);
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
    chunk = (int *)malloc(chunk_size * sizeof(int));
    MPI_Scatter(data, chunk_size, MPI_INT, chunk, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    int global_pivot = calculate_pivot(chunk, chunk_size, id, p, pivot_strategy, MPI_COMM_WORLD);
    double start_time = MPI_Wtime();
    quicksort(chunk, 0, chunk_size - 1, id, p, pivot_strategy, MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if(id == 0){
        printf("time:%lf \n", end_time-start_time);
    }
    // Tree-based merge
    int step = 1;
    MPI_Status status;
    while (step < p) {
        if (id % (2 * step) == 0) {
            int sender = id + step;
            if (sender < p) {
                int received_size;
                MPI_Recv(&received_size, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, &status);
                other = (int *)malloc(received_size * sizeof(int));
                MPI_Recv(other, received_size, MPI_INT, sender, 0, MPI_COMM_WORLD, &status);
                temp = merge(chunk, chunk_size, other, received_size);
                free(chunk);
                free(other);
                chunk = temp;
                chunk_size += received_size;
            }
        } else {
            int receiver = id - step;
            MPI_Send(&chunk_size, 1, MPI_INT, receiver, 0, MPI_COMM_WORLD);
            MPI_Send(chunk, chunk_size, MPI_INT, receiver, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    // Final output at root
    if (id == 0) {
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
    //print time

    return 0;
}
