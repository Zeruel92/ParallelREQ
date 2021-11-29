#include <iostream>
#include <fstream>
#include <ctime>

typedef float data_t;

int main(int argc, char* argv[]) {
    int i;
    long n;
    data_t *a;

    if(argc < 3){
        std::cerr<<"Missing input param"<<std::endl;
        std::cerr<<"Usage ./bin_generator <n> <path>"<<std::endl;
        return -1;
    }
    std::ofstream fout(argv[2], std::fstream::trunc | std::ios::binary);

    std::srand(std::time(NULL));

    n = strtol (argv[1],NULL,10);
    std::cout<<"Generating "<<n<<" elements with uniform distribution"<<std::endl;
    a = (data_t *) malloc(n * sizeof(data_t));
    for (i = 0; i < n; i++) {
        a[i] = (float) (i) / (float) (n);
        //a[i] = (data_t) i *  ((data_t) std::rand() /RAND_MAX)/ (data_t) n;
        //std::cout << a[i] << std::endl;
    }

    for (i = 0; i < n; i++)
    {
        //size_t j = i + rand() / (RAND_MAX / (len - i) + 1);
        int j = i +(rand()%(n-i));
        float t = a[j];
        a[j] = a[i];
        a[i] = t;
        //fprintf(stdout,"%d ",array[i]);
    }

    //fout.open("streamdata");
    fout.write((char *) &n, sizeof(long));
    fout.write((char *) a, sizeof(data_t) * n);

    fout.close();
    return 0;
}
