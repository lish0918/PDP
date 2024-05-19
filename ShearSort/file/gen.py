import random

def generate_matrix(n):
    matrix = [[random.randint(1, 100) for _ in range(n)] for _ in range(n)]
    return matrix

def write_matrix_to_file(matrix, filename):
    with open(filename, 'w') as file:
        for row in matrix:
            file.write(' '.join(map(str, row)) + '\n')

n = 1000
matrix = generate_matrix(n)
write_matrix_to_file(matrix, 'input1000.txt')