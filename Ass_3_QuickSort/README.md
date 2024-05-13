# QuickSort

- [快排原理](https://blog.csdn.net/qq_39181839/article/details/109478094)，时间复杂度O(nlogn)
- [MPI使用](https://stackoverflow.com/questions/36249371/mpi-quicksort-program)

### directory
cd ../../../../../../proj/uppmax2024-2-9/nobackup/A3

### run it
mpirun -np 4 ./quicksort ../../../../../../../proj/uppmax2024-2-9/nobackup/A3/inputs/input10.txt result.txt 2

### store
- ls inputs
input1000000000.txt  input125000000.txt   input250000000.txt  input4.txt          input93.txt            input_backwards3.txt  input_backwards93.txt
input10.txt          input2000000000.txt  input3.txt          input500000000.txt  input_backwards10.txt  input_backwards4.txt  Usersemfo7814

- ls outputs
output10.txt  output125000000.txt  output250000000.txt  output3.txt  output4.txt  output93.txt

### interact snowy
interactive -M snowy -A uppmax2024-2-9 -n 16 -t 01:10:00
module load gcc/12.2.0 openmpi/4.1.4
