def check_sorted(file_path):
    with open(file_path, 'r') as file:
        numbers = [int(num) for num in file.read().split()]
        length = len(numbers)
        sorted_result = all(numbers[i] <= numbers[i+1] for i in range(length-1))
        return sorted_result, length

file_path = '/home/lish6557/local/src/PDP/Ass_3_QuickSort/result.txt'
is_sorted, array_length = check_sorted(file_path)

if is_sorted:
    print(f"The sequence of length {array_length} is sorted in ascending order.")
else:
    print(f"The sequence of length {array_length} is not sorted in ascending order.")
