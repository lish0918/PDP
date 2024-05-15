## Stencil MPI Parallel

The `stencil` MPI program is used for performing stencil-based computations. Below are examples of how to run the program with different input parameters.

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

### Input Files

The input files required for running the `stencil` MPI program are located in the following directory path:

```zsh
cd ../../../../../../proj/uppmax2024-2-9/A2/
```

#### List of Input Files

- `input1000000.txt`
- `input120.txt`
- `input2000000.txt`
- `input4000000.txt`
- `input8000000.txt`
- `input96.txt`
- `output96_4_ref.txt`

### Execution

#### 1. Compile It

```zsh
make stencil
```

#### 2. Run It

```zsh
mpirun -np <num_processes> ./stencil <input_file> <output_file> <iterations>
```

- `<num_processes>`: Number of MPI processes to be used.
- `<input_file>`: Path to the input file containing the matrix data.
- `<output_file>`: Path to the output file where the results will be saved.
- `<iterations>`: Number of iterations to run the stencil computation.

### Examples

#### Example 1: Running with `input1000000.txt` and `output96_4_ref.txt`

```zsh
mpirun -np 1 ./stencil ../../../../../../../proj/uppmax2024-2-9/A2/input1000000.txt output96_4_ref.txt 10
```

This command runs the `stencil` program using 1 MPI process (`-np 1`) with the input file `input1000000.txt` located at `../../../../../../../proj/uppmax2024-2-9/A2/`. The computed results will be saved to `output96_4_ref.txt`, and the stencil computation will iterate 10 times (`<iterations>`).

#### Example 2: Running with `input96.txt` and `output96_4_ref.txt`

```zsh
mpirun -np 1 ./stencil ../../../../../../../proj/uppmax2024-2-9/A2/input96.txt ../../../../../../../scratch/uppmax2024-2-9/output96_4_ref.txt 4
```

This command similarly runs the `stencil` program using 1 MPI process (`-np 1`) with the input file `input96.txt` located at `../../../../../../../proj/uppmax2024-2-9/A2/`. The computed results will be saved to `output96_4_ref.txt`, and the stencil computation will iterate 4 times (`<iterations>`).