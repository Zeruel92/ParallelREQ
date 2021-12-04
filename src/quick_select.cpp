#include <bits/stdc++.h>

typedef float data_t;

int main(int argc, char *argv[]){
    std::ifstream inputStream;
    long n;
    data_t *elements= nullptr;
    data_t quantile;
    int k;
    float ranks[12] = {0.01, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.99};

    inputStream.open(argv[1], std::ios::binary);
    inputStream.read((char *) &n,sizeof(long));
    elements = (data_t *) malloc(sizeof(data_t)*n);
    inputStream.read((char *)elements,sizeof(data_t)*n);
    inputStream.close();

    std::ofstream fout(argv[2], std::fstream::trunc | std::ios::binary);
    for(int i =0; i < 12; i++){
        k = floor(ranks[i]*n)+1;
        std::nth_element(elements,elements+k,elements+n);
        quantile = elements[k];
        std::cout<<"Rank "<<ranks[i]<<" Quantile: "<<quantile<<std::endl;
        fout.write((char *) &quantile, sizeof(data_t));
    }
    fout.close();
    free(elements),elements= nullptr;
    return 0;
}