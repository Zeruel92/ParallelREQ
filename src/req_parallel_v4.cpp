#include <mpi.h>
#include <iostream>
#include <fstream>
#include <req_sketch.hpp>

#define BLOCK_LOW(id, p, n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id, p, n) \
                     (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

typedef float data_t;
bool hra = true;
int k = 12;
void merge_reduce(void *inputBuffer, void *outputBuffer,int *len,MPI_Datatype *datatype);

typedef struct {
    size_t size;
    uint8_t *data;
}serialized_t;

int main(int argc, char** argv){
    int processes, rank;
    int iterations;
    double elapsed,global_elapsed;
    std::ifstream inputStream;
    long n;
    data_t *elements= nullptr;
    int block_size;
    int block_low;
    datasketches::req_sketch<float> sketch(k);
    int B,number_compactors,max_data_size;
    MPI_Op merge_reduce_op;
    serialized_t *serialized_sketch= nullptr;
    serialized_t *merged_sketch= nullptr;
    MPI_Datatype mpi_serialized_sketch;
    data_t *ground_truth = nullptr;
    double ranks[12] = {0.01, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.99};
    std::vector<float> quantiles;
    data_t *accuracy = nullptr;

#ifdef _DEBUG
    int debug = 1;
    MPI_Status status;
#endif
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&processes);

    if(argc < 4){
        std::cerr<<"Usage: mpirun --np <#process> ./"<<argv[1]<<" <input vector> <ground_truth> <1/0 for hra>"<<std::endl;
        MPI_Abort(MPI_COMM_WORLD,-1);
    }
    hra = strtol(argv[3],NULL,10);

    MPI_Op_create(merge_reduce,0,&merge_reduce_op);

    inputStream.open(argv[1], std::ios::binary);
    inputStream.read((char *) &n,sizeof(long));

    block_size = BLOCK_SIZE(rank,processes,n);
    block_low = BLOCK_LOW(rank,processes,n);

    B = 2*k* ceil(log2(n/k));
    number_compactors = ceil(log2(n/B)) +1;
    max_data_size = number_compactors*B*sizeof(data_t);

    serialized_sketch =(serialized_t *) malloc(sizeof(serialized_t));
    merged_sketch = (serialized_t *) malloc(sizeof(serialized_t));
    memset(serialized_sketch,0,sizeof(serialized_t));
    serialized_sketch->data =(uint8_t *) calloc(max_data_size,sizeof(uint8_t));
    memset(merged_sketch,0,sizeof(serialized_t));
    merged_sketch->data =(uint8_t *) calloc(max_data_size,sizeof(uint8_t));

    int lengths[2] = {1,max_data_size};
    MPI_Aint displacements[2];
    MPI_Aint base_address;
    MPI_Get_address(serialized_sketch,&base_address);
    MPI_Get_address(&serialized_sketch->size,&displacements[0]);
    MPI_Get_address(serialized_sketch->data,&displacements[1]);
    displacements[0] = MPI_Aint_diff(displacements[0],base_address);
    displacements[1] = MPI_Aint_diff(displacements[1],base_address);
    MPI_Datatype types[2] = {MPI_UNSIGNED_LONG,MPI_BYTE};
    MPI_Type_create_struct(2,lengths,displacements,types,&mpi_serialized_sketch);
    MPI_Type_commit(&mpi_serialized_sketch);

    elements = (data_t *) malloc(sizeof(data_t)*block_size);

    inputStream.seekg(block_low*sizeof(data_t),std::ios_base::cur);

    inputStream.read((char *)elements,sizeof(data_t)*block_size);
    inputStream.close();
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
        sketch = datasketches::req_sketch<data_t>(12,hra);
        for(int i=0; i < block_size; i ++){
            sketch.update(elements[i]);
        }
        std::vector<uint8_t, std::allocator<uint8_t>> bytes = sketch.serialize();
        int data_size = bytes.size();

#ifdef _DEBUG
        std::cout<<"Process "<<rank<<" Bytes "<<data_size<<" Max Bytes "<< max_data_size<<" Block size "<<block_size<<std::endl;
#endif

        serialized_sketch->size = bytes.size();
        memcpy(serialized_sketch->data,bytes.data(),serialized_sketch->size);

        std::cout<<"Ser sketch addr: "<<serialized_sketch<<" Merg sket addr"<< merged_sketch<<std::endl;

        MPI_Reduce(serialized_sketch,merged_sketch,1,mpi_serialized_sketch,merge_reduce_op,0,MPI_COMM_WORLD);
        if(!rank) sketch = datasketches::req_sketch<data_t>::deserialize(merged_sketch->data,merged_sketch->size);
        free(merged_sketch->data),merged_sketch->data= nullptr;
        free(serialized_sketch->data),serialized_sketch->data= nullptr;
    }
    free(elements),elements= nullptr;

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed += MPI_Wtime();
    elapsed /= MAX_ITERATIONS;
    MPI_Reduce(&elapsed,&global_elapsed,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Finalize();

    if(!rank) {
        std::cout << "ParallelREQ running time:" << global_elapsed << " number of processors: " << processes << std::endl;


        quantiles = sketch.get_quantiles(ranks, 12);
        inputStream.open(argv[2], std::ios::binary);
        ground_truth = (data_t *) malloc(sizeof(data_t)*12);
        accuracy = (data_t * ) malloc(sizeof(data_t)*12);
        inputStream.read((char *) ground_truth, sizeof(data_t)*12);

        for (int i = 0; i < 12; i++) {
            accuracy[i] = 100 - (std::abs(quantiles[i]-ground_truth[i])/quantiles[i] * 100);
            std::cout << "Rank: " << ranks[i] << " Quantile: " << quantiles[i] <<" Accuracy: "<<accuracy[i]<<"%"<<std::endl;
        }
    }
    free(accuracy), accuracy= nullptr;
    free(ground_truth),ground_truth = nullptr;
    return 0;
}

void merge_reduce(void *inputBuffer, void *outputBuffer,int *len,MPI_Datatype *datatype){
    datasketches::req_sketch<data_t> insketch(k,hra);
    datasketches::req_sketch<data_t> outsketch(k,hra);
     serialized_t *serialized_input = ( serialized_t *) inputBuffer;
     serialized_t *serialized_output = ( serialized_t *) outputBuffer;
    insketch = datasketches::req_sketch<data_t>::deserialize(serialized_input->data, serialized_input->size);
    outsketch = datasketches::req_sketch<data_t>::deserialize(serialized_output->data, serialized_output->size);
    outsketch.merge(insketch);
    std::vector<uint8_t, std::allocator<uint8_t>> bytes = outsketch.serialize();
    serialized_output->size = bytes.size();
    memcpy((uint8_t *) serialized_output->data,bytes.data(),bytes.size());
}
//TODO: eliminare la all reduce, usare la formula per determinare la dimensione massima dell'array
//TODO: creare un'array che contenga la dimensione effettiva e la serializzazione dello sketch
//TODO: deserializzare tenendo conto della dimensione scritta nell'array
//TODO: creare un tipo di dato contigous per usare un solo elemento nella reduce//

