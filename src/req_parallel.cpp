#include <mpi.h>

int main(int argc, char** argv){
    int processes, rank;
    int iterations;
    double elapsed,global_elapsed;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&processes);

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed = -MPI_Wtime();
    for(iterations=0; iterations<MAX_ITERATIONS; iterations++){

    }
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed += MPI_Wtime();
    elapsed /= MAX_ITERATIONS;

    MPI_Reduce(&elapsed,&global_elapsed,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}