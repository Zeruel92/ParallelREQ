#include <mpi.h>
#include <iostream>
#include <fstream>

#define MIN(a, b)           ((a)<(b)?(a):(b))
#define BLOCK_LOW(id, p, n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id, p, n) \
                     (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
#define BLOCK_OWNER(j, p, n) (((p)*((j)+1)-1)/(n))
#define PTR_SIZE           (sizeof(void*))
#define CEILING(i, j)       (((i)+(j)-1)/(j))

typedef float data_t;


int main(int argc, char** argv){
    int processes, rank;
    int iterations;
    double elapsed,global_elapsed;
    std::ifstream inputStream;
    long n;
    data_t *elements=NULL;
    int block_size;
    int block_low,block_high;

#ifdef _DEBUG
    int debug = 1;
    MPI_Status status;
#endif
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&processes);

    if(argc < 2){
        std::cerr<<"Missing input file"<<std::endl;
        MPI_Abort(MPI_COMM_WORLD,-1);
    }

    inputStream.open(argv[1], std::ios::binary);
    inputStream.read((char *) &n,sizeof(long));

    block_size = BLOCK_SIZE(rank,processes,n);
    block_high = BLOCK_HIGH(rank,processes,n);
    block_low = BLOCK_LOW(rank,processes,n);

    elements = (data_t *) malloc(sizeof(data_t)*block_size);

    inputStream.seekg(block_low*sizeof(data_t),std::ios_base::cur);

    inputStream.read((char *)elements,sizeof(data_t)*block_size);

#ifdef _DEBUG
    if (!rank) {
        std::cout<<"INPUT Vector: "<<n<<std::endl;
    } else {
        MPI_Recv(&debug, 1, MPI_INT, (rank - 1) % processes, 1, MPI_COMM_WORLD, &status);
    }
    for (int i = 0; i < block_size; i++) {
        std::cout<<elements[i]<<std::endl;
    }
    if (processes > 1) MPI_Send(&debug, 1, MPI_INT, (rank + 1) % processes, 1, MPI_COMM_WORLD);
#endif

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed = -MPI_Wtime();
    for(iterations=0; iterations<MAX_ITERATIONS; iterations++){

    }
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed += MPI_Wtime();
    elapsed /= MAX_ITERATIONS;

    MPI_Reduce(&elapsed,&global_elapsed,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    std::cout<<"ParallelREQ time:"<<global_elapsed<<" number of processors: "<<processes<<std::endl;
    MPI_Finalize();

    free(elements),elements=NULL;
    return 0;
}