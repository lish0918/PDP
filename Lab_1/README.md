**to get the interactive session on snowy**
- interactive -M snowy -A uppmax2024-2-9 -n 2 -t 01:10:00

**load the compiler and the library**
- module load gcc/12.2.0 openmpi/4.1.4

**to compile**
- mpicc ..

**to run**
- mpirun -n 2 ./a.out
