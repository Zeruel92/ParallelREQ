#include <mpi.h>
#include <iostream>
#include <fstream>
#include <req_sketch.hpp>

#define BLOCK_LOW(id, p, n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id, p, n) \
                     (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

typedef float data_t;

void merge_reduce(void *inputBuffer, void *outputBuffer,int *len,MPI_Datatype *datatype);

int main(int argc, char** argv){
    int processes, rank;
    int iterations;
    double elapsed,global_elapsed;
    std::ifstream inputStream;
    long n;
    data_t *elements=NULL;
    int block_size;
    int block_low,block_high;
    datasketches::req_sketch<float> sketch(12);
    MPI_Op merge_reduce_op;
    uint8_t *merged_bytes;

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

    MPI_Op_create(merge_reduce,0,&merge_reduce_op);

    inputStream.open(argv[1], std::ios::binary);
    inputStream.read((char *) &n,sizeof(long));

    block_size = BLOCK_SIZE(rank,processes,n);
    block_high = BLOCK_HIGH(rank,processes,n);
    block_low = BLOCK_LOW(rank,processes,n);

    elements = (data_t *) malloc(sizeof(data_t)*block_size);

    inputStream.seekg(block_low*sizeof(data_t),std::ios_base::cur);

    inputStream.read((char *)elements,sizeof(data_t)*block_size);

#ifdef DEBUG
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
        for(int i=0; i < block_size; i ++){
            sketch.update(elements[i]);
        }
        std::vector<uint8_t, std::allocator<uint8_t>> bytes = sketch.serialize();
        int data_size = bytes.size();
        merged_bytes =(uint8_t *) calloc(data_size,sizeof(uint8_t));
        MPI_Reduce(bytes.data(),merged_bytes,data_size,MPI_BYTE,merge_reduce_op,0,MPI_COMM_WORLD);

        if(!rank) sketch = datasketches::req_sketch<data_t>::deserialize((uint8_t *) merged_bytes,(size_t) data_size);
        free(merged_bytes),merged_bytes=NULL;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed += MPI_Wtime();
    elapsed /= MAX_ITERATIONS;
    MPI_Reduce(&elapsed,&global_elapsed,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Finalize();

    if(!rank) {
        std::cout << "ParallelREQ time:" << global_elapsed << " number of processors: " << processes << std::endl;

        double ranks[12] = {0.01, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.99};
        std::vector<float> quantiles;
        quantiles = sketch.get_quantiles(ranks, 12);

        for (int i = 0; i < 12; i++) {
            std::cout << "Rank: " << ranks[i] << " Quantile: " << quantiles[i] << std::endl;
        }
    }
    free(elements),elements=NULL;
    return 0;
}

void merge_reduce(void *inputBuffer, void *outputBuffer,int *len,MPI_Datatype *datatype){
    datasketches::req_sketch<data_t> insketch(12);
    datasketches::req_sketch<data_t> outsketch(12);
    insketch = datasketches::req_sketch<data_t>::deserialize((uint8_t *) inputBuffer,(size_t) len);
    outsketch = datasketches::req_sketch<data_t>::deserialize((uint8_t *) outputBuffer,(size_t) len);
    outsketch.merge(insketch);
    std::vector<uint8_t, std::allocator<uint8_t>> bytes = outsketch.serialize();
    outputBuffer = (void *) bytes.data();
}