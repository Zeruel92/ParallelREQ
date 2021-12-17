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

void merge_reduce(void *inputBuffer, void *outputBuffer, int *len, MPI_Datatype *datatype);

int main(int argc, char **argv) {
    int processes, rank;
    int iterations;
    double elapsed, global_elapsed;
    std::ifstream inputStream;
    long n;
    data_t *elements = nullptr;
    int block_size;
    int block_low;
    datasketches::req_sketch<float> sketch(k);
    int B, number_compactors, max_data_size;
    MPI_Op merge_reduce_op;
    uint8_t *serialized_sketch = nullptr;
    uint8_t *merged_sketch = nullptr;
    data_t *ground_truth = nullptr;
    double ranks[12] = {0.01, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.99};
    std::vector<float> quantiles;
    data_t *accuracy = nullptr;

#ifdef _DEBUG
    int debug = 1;
    MPI_Status status;
#endif
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    if (argc < 4 && !rank) {
        std::cerr << "Usage: mpirun --np <#process>" << argv[0] << " <input vector> <ground_truth> <1/0 for hra>"
                  << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    hra = strtol(argv[3], NULL, 10);

    MPI_Op_create(merge_reduce, 0, &merge_reduce_op);

    inputStream.open(argv[1], std::ios::binary);
    if(!inputStream){
        std::cerr<<"Error opening file "<<argv[1]<<std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }
    inputStream.read((char *) &n, sizeof(long));

    block_size = BLOCK_SIZE(rank, processes, n);
    block_low = BLOCK_LOW(rank, processes, n);

    B = 2 * k * ceil(log2(n / k));
    number_compactors = ceil(log2(n / B)) + 1;

    max_data_size = number_compactors * B * sizeof(data_t);

    int buffersize = max_data_size + sizeof(size_t);

    elements = (data_t *) malloc(sizeof(data_t) * block_size);

    inputStream.seekg(block_low * sizeof(data_t), std::ios_base::cur);

    inputStream.read((char *) elements, sizeof(data_t) * block_size);
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
    for (iterations = 0; iterations < MAX_ITERATIONS; iterations++) {
        sketch = datasketches::req_sketch<data_t>(12, hra);
        for (int i = 0; i < block_size; i++) {
            sketch.update(elements[i]);
        }
        serialized_sketch = (uint8_t *) malloc(buffersize);
        if(serialized_sketch == nullptr){
            std::cerr<<"Not Enough memory to alloc"<<std::endl;
            MPI_Abort(MPI_COMM_WORLD,-3);
        }
        merged_sketch = (uint8_t *) malloc(buffersize);
        if(merged_sketch == nullptr){
            std::cerr<<"Not Enough memory to alloc"<<std::endl;
            MPI_Abort(MPI_COMM_WORLD,-3);
        }

        std::vector<uint8_t, std::allocator<uint8_t>> bytes = sketch.serialize();
        size_t data_size = bytes.size();

#ifdef _DEBUG
        std::cout<<"Process "<<rank<<" Bytes "<<data_size<<" Max Bytes "<< max_data_size<<" Block size "<<block_size<<std::endl;
#endif

        memcpy(serialized_sketch + sizeof(size_t), bytes.data(), data_size);
        memcpy(serialized_sketch, &data_size, sizeof(size_t));

        MPI_Reduce(serialized_sketch, merged_sketch, buffersize, MPI_BYTE, merge_reduce_op, 0, MPI_COMM_WORLD);
        if (!rank) {
            data_size = *(size_t *) merged_sketch;
            sketch = datasketches::req_sketch<data_t>::deserialize(merged_sketch + sizeof(size_t), data_size);
        }
        free(merged_sketch), merged_sketch = nullptr;
        free(serialized_sketch), serialized_sketch = nullptr;
    }
    free(elements), elements = nullptr;

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed += MPI_Wtime();
    elapsed /= MAX_ITERATIONS;
    MPI_Reduce(&elapsed, &global_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Finalize();

    if (!rank) {
        std::cout << "ParallelREQ running time:" << global_elapsed << " number of processors: " << processes
                  << std::endl;

        quantiles = sketch.get_quantiles(ranks, 12);
        inputStream.open(argv[2], std::ios::binary);
        if(!inputStream){
            std::cerr<<"Error opening file "<<argv[2]<<std::endl;
            return -2;
        }
        ground_truth = (data_t *) malloc(sizeof(data_t) * 12);
        if(ground_truth == nullptr){
            std::cerr<<"Not Enough memory to alloc"<<std::endl;
            return -3;
        }
        accuracy = (data_t *) malloc(sizeof(data_t) * 12);
        if(accuracy == nullptr){
            std::cerr<<"Not Enough memory to alloc"<<std::endl;
            return -3;
        }
        inputStream.read((char *) ground_truth, sizeof(data_t) * 12);

        for (int i = 0; i < 12; i++) {
            accuracy[i] = 100 - (std::abs(quantiles[i] - ground_truth[i]) / quantiles[i] * 100);
            std::cout << "Rank: " << ranks[i] << " Quantile: " << quantiles[i] << " Accuracy: " << accuracy[i] << "%"
                      << std::endl;
        }
    }
    free(accuracy), accuracy = nullptr;
    free(ground_truth), ground_truth = nullptr;
    return 0;
}

void merge_reduce(void *inputBuffer, void *outputBuffer, int *len, MPI_Datatype *datatype) {
    datasketches::req_sketch<data_t> insketch(k, hra);
    datasketches::req_sketch<data_t> outsketch(k, hra);

    size_t insize = *(size_t *) inputBuffer;
    size_t outsize = *(size_t *) outputBuffer;

    insketch = datasketches::req_sketch<data_t>::deserialize((uint8_t *) inputBuffer + sizeof(size_t), insize);
    outsketch = datasketches::req_sketch<data_t>::deserialize((uint8_t *) outputBuffer + sizeof(size_t), outsize);

    outsketch.merge(insketch);

    std::vector<uint8_t, std::allocator<uint8_t>> bytes = outsketch.serialize();
    outsize = bytes.size();
    memcpy((uint8_t *) outputBuffer, &outsize, sizeof(size_t));
    memcpy((uint8_t *) outputBuffer + sizeof(size_t), bytes.data(), bytes.size());
}

