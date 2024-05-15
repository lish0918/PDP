#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 2000000001

int compare(const void *a, const void *b) {
    return (*(int*)b - *(int*)a);
}

int main() {
    FILE *inputFile, *outputFile;
    int numbers[MAX_SIZE];
    int count = 0;
    int totalLength;

    // Open the input file
    inputFile = fopen("../../../../../../../proj/uppmax2024-2-9/nobackup/A3/inputs/input125000000.txt", "r");
    if (inputFile == NULL) {
        printf("Failed to open input file.\n");
        return 1;
    }

    // Read the total length (first number in the file)
    if (fscanf(inputFile, "%d", &totalLength) != 1) {
        printf("Failed to read total length from input file.\n");
        fclose(inputFile);
        return 1;
    }

    // Read the numbers from the input file
    while (count < totalLength && fscanf(inputFile, "%d", &numbers[count]) == 1) {
        count++;
    }

    // Close the input file
    fclose(inputFile);

    // Sort the numbers in reverse order
    qsort(numbers, count, sizeof(int), compare);

    // Open the output file
    outputFile = fopen("./reversed.txt", "w");
    if (outputFile == NULL) {
        printf("Failed to open output file.\n");
        return 1;
    }

    // Write the total length as the first number in the output file
    fprintf(outputFile, "%d\n", totalLength);

    // Write the sorted numbers to the output file
    for (int i = 0; i < count; i++) {
        fprintf(outputFile, "%d\n", numbers[i]);
    }

    // Close the output file
    fclose(outputFile);

    printf("Sorting and writing to reversed.txt completed successfully.\n");

    return 0;
}
