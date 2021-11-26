#include <req_sketch.hpp>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]){

    if(argc < 2){
        std::cerr<<"Missing input file"<<std::endl;
        return -1;
    }

    int n;
    std::ifstream is (argv[1], std::ios::binary);
    datasketches::req_sketch<float> sketch(12);

#if 0
    //load sketch from file
    std::ifstream sketchfile("quantile",std::ios::binary);

    //serialize deserialize in memery
    sketch = datasketches::req_sketch<float>::deserialize(sketchfile);
    std::vector<uint8_t, std::allocator<uint8_t>> bytes = sketch.serialize();
    sketch = datasketches::req_sketch<float>::deserialize(bytes.data(),bytes.size());
#endif
#if 1
    is.read((char *) &n,sizeof(int));

    //std::cout<<n;
    float element;
    //build sketch
    for (int i = 0; i <n; i++){
        is.read((char *)&element,sizeof(float));
       // std::cout<<element<<std::endl;
        sketch.update(element);
    }
#endif

    //get quantiles
    double ranks[12] = {0.01, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.99};
    std::vector<float> quantiles;
    quantiles = sketch.get_quantiles(ranks,12);

    for(int i =0 ; i <12; i++){
        std::cout<<"Rank: "<<ranks[i]<<" Quantile: "<<quantiles[i]<< std::endl;
    }
#if 0

    std::ofstream quantilefile("quantile", std::ios::trunc| std::ios::binary);
    sketch.serialize(quantilefile);
#endif
    return 0;
}
