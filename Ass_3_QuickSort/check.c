#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool check_sorted(const char* file_path, int *length) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int num;
    int count = 0;
    int capacity = 10;
    int *numbers = (int*) malloc(capacity * sizeof(int));
    if (!numbers) {
        perror("Memory allocation failed");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%d", &num) != EOF) {
        if (count >= capacity) {
            capacity *= 2;
            numbers = (int*) realloc(numbers, capacity * sizeof(int));
            if (!numbers) {
                perror("Memory reallocation failed");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
        numbers[count++] = num;
    }
    fclose(file);

    *length = count;
    bool sorted = true;
    int i;  // Declare the variable outside the loop
    for (i = 0; i < count - 1; i++) {
        if (numbers[i] > numbers[i + 1]) {
            sorted = false;
            break;
        }
    }

    free(numbers);
    return sorted;
}

int main() {
    const char *file_path = "/home/lish6557/local/src/PDP/Ass_3_QuickSort/result.txt";
    int array_length;
    bool is_sorted = check_sorted(file_path, &array_length);

    if (is_sorted) {
        printf("The sequence of length %d is sorted in ascending order.\n", array_length);
    } else {
        printf("The sequence of length %d is not sorted in ascending order.\n", array_length);
    }

    return 0;
}
