#include <mpi.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
    int myid, numprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Status stat;

    int count = 0, max_iter = 10, partner = (myid + 1) % 2;
    while (count < max_iter) {
        if (myid == count % 2) {
            count++;
            MPI_Send(&count, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            printf("%d sent the counter %d to %d\n", myid, count, partner);
        } else {
            MPI_Recv(&count, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, &stat);
            printf("%d received the counter %d from %d\n", myid, count, partner);
        }
    }

    MPI_Finalize();
    return 0;
}