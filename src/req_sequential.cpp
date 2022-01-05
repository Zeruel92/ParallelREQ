#include <req_sketch.hpp>
#include <iostream>
#include <fstream>

typedef float data_t;

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "Missing input file" << std::endl;
        return -1;
    }

    int n;
    std::ifstream is(argv[1], std::ios::binary);
    datasketches::req_sketch<float> sketch(12,false);
    float *ground_truth;
    float *accuracy;
#if 0
    //load sketch from file
    std::ifstream sketchfile("quantile",std::ios::binary);

    //serialize deserialize in memery
    sketch = datasketches::req_sketch<float>::deserialize(sketchfile);
    std::vector<uint8_t, std::allocator<uint8_t>> bytes = sketch.serialize();
    sketch = datasketches::req_sketch<float>::deserialize(bytes.data(),bytes.size());
#endif
#if 1
    is.read((char *) &n, sizeof(int));

    //std::cout<<n;
    float *element;
    element = (float *) malloc(sizeof(float) * n);
    //build sketch
    is.read((char *) element, sizeof(float) * n);
    is.close();
    auto start = std::chrono::steady_clock::now();
    for(int k = 0; k< MAX_ITERATIONS; k++) {
        sketch = datasketches::req_sketch<data_t>(12, false);
        for (int i = 0; i < n; i++)
            sketch.update(element[i]);
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed = (double) std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()/n;
    elapsed/=MAX_ITERATIONS;
    std::cout << "One update operation took: = " << elapsed << "[ns]" << std::endl;
    free(element), element = nullptr;
#endif

    //get quantiles
    double ranks[12] = {0.01, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.99};
    std::vector<float> quantiles;
    quantiles = sketch.get_quantiles(ranks, 12);
    is.open(argv[2], std::ios::binary);
    ground_truth = (data_t *) malloc(sizeof(data_t) * 12);
    accuracy = (data_t *) malloc(sizeof(data_t) * 12);
    is.read((char *) ground_truth, sizeof(data_t) * 12);
    for (int i = 0; i < 12; i++) {
        accuracy[i] = 100 - (std::abs(quantiles[i] - ground_truth[i]) / ground_truth[i] * 100);
        std::cout << "Rank: " << ranks[i] << " Quantile: " << quantiles[i] <<" Accuracy:"<< accuracy[i] << std::endl;
    }
#if 0

    std::ofstream quantilefile("quantile", std::ios::trunc| std::ios::binary);
    sketch.serialize(quantilefile);
#endif
    free(ground_truth),ground_truth= nullptr;
    free(accuracy),accuracy= nullptr;
    return 0;
}
