## QuickSort with MPI

This guide provides instructions for running a QuickSort program using MPI on Uppmax.

### QuickSort Overview

QuickSort is a sorting algorithm known for its efficiency, with a time complexity of O(nlogn). To learn more about the QuickSort principle, refer to [this article](https://blog.csdn.net/qq_39181839/article/details/109478094).

### MPI Implementation Reference

For an MPI implementation of QuickSort, you can refer to [this Stack Overflow thread](https://stackoverflow.com/questions/36249371/mpi-quicksort-program) for code examples and insights into using MPI for parallel sorting.

### Environment

### Environment Configuration for Uppmax (Snowy)

#### 1. Connect via VS Code SSH Remote

Use Visual Studio Code (VS Code) to connect to Uppmax via SSH remote access.

#### 2. Access Snowy Interactive Session

```zsh
interactive -M snowy -A uppmax2024-2-9 -n 16 -t 04:10:00
```

- `-M snowy`: Specifies the compute cluster (Snowy).
- `-A uppmax2024-2-9`: Uses the Uppmax project allocation (`uppmax2024-2-9`).
- `-n 16`: Allocates 16 CPU cores for the session.
- `-t 04:10:00`: Sets the session time limit to 4 hours and 10 minutes.

#### 3. Load Required Modules

```zsh
module load gcc/12.2.0 openmpi/4.1.4
```

### Directory

Navigate to the QuickSort program directory:

```bash
cd ../../../../../../proj/uppmax2024-2-9/nobackup/A3
```

### Input and Output Files

#### Input Files

```bash
ls inputs
input1000000000.txt  input125000000.txt   input250000000.txt  input4.txt          input93.txt            input_backwards3.txt  input_backwards93.txt
input10.txt          input2000000000.txt  input3.txt          input500000000.txt  input_backwards10.txt  input_backwards4.txt
```

#### Output Files

```bash
ls outputs
output10.txt  output125000000.txt  output250000000.txt  output3.txt  output4.txt  output93.txt
```

### Running the QuickSort Program

To execute the QuickSort program using MPI, use the following command:

```bash
mpirun -np 4 ./quicksort ../../../../../../../proj/uppmax2024-2-9/nobackup/A3/inputs/input10.txt result.txt 2
```

- `-np 4`: Specifies 4 MPI processes for parallel execution.
- `./quicksort`: Executable binary for the QuickSort program.
- `../../../../../../../proj/uppmax2024-2-9/nobackup/A3/inputs/input10.txt`: Path to the input file (`input10.txt`) containing the data to be sorted.
- `result.txt`: Path to the output file where the sorted result will be saved.
- `2`: Pivot Strategy.