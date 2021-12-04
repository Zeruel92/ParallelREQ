#include <bits/stdc++.h>

typedef float data_t;

int partition(data_t arr[], int low, int high)
{
    data_t pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}

// Implementation of QuickSelect
float quick_select(data_t a[], int left, int right, int k)
{
    while (left <= right) {
        int pivotIndex = partition(a, left, right);
        if (pivotIndex == k - 1)
            return a[pivotIndex];
        else if (pivotIndex > k - 1)
            right = pivotIndex - 1;
        else
            left = pivotIndex + 1;
    }
    return -1;
}

int main(int argc, char *argv[]){
    std::ifstream inputStream;
    long n;
    data_t *elements= nullptr;
    inputStream.open(argv[1], std::ios::binary);
    inputStream.read((char *) &n,sizeof(long));
    elements = (data_t *) malloc(sizeof(data_t)*n);
    inputStream.read((char *)elements,sizeof(data_t)*n);
    inputStream.close();
    std::ofstream fout(argv[2], std::fstream::trunc | std::ios::binary);
    float ranks[12] = {0.01, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.99};
    for(int i =0; i < 12; i++){
        data_t quantile = quick_select(elements,0,n-1,ranks[i]*n);
        std::cout<<"Rank "<<ranks[i]<<" Quantile: "<<quantile<<std::endl;
        fout.write((char *) &quantile, sizeof(data_t));
    }
    fout.close();
    free(elements),elements= nullptr;
    return 0;
}