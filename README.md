# ParallelREQ

ParallelREQ is an MPI parallel implementation of the REQ algorithm for computing quantiles in streaming of data

The sequential code used can be found at: https://datasketches.apache.org/docs/Community/Downloads.html

## Project Structure

### bin_generator

`bin_generator` is a utility to generate binary file with `n` numbers in uniform distribution `[0,1)`

Usage: `./bin_generator <n> <outputfile>`

The output file has as first element the number of total elements stored, and following all the others elements.

### intro_select

Perform the IntroSelect algorithm on the input file providing a ground truth output file used for compute the accuracy of ParallelREQ

Usage: `./intro_select <input file> <output file>`

### req_sequential

`req_sequential` uses the apache datasketches library to compute quantiles in sequential

### req_parallel

`req_parallel` uses the `Apache datasketches` library to compute quantiles in parallel.
Usage: `mpirun --np <number of processes> ./req_parallel <fileinput> <ground_truth> <0/1 hra>`
Each process read the file in input to retrieve the total number of elements `n`.
After that each process compute the own block of data and read it from file.

Once every process has finished read the data, using the `Apache datasketches` they compute the intermediate summary.

Finally, using a user defined reduction the intermediate summaries are merged into one.

### benchmark.sh

Is a simple script that execute req_parallel with a number of processes ranging from 1 to 4,
in order to take some performance measure

## Compiling

Using cmake 
```
mkdir cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=<BUILD_TYPE> ..
make
```
 
***BUILD_TYPE*** can be:

- ***Release***: cmake will use this flags for compiling: `-O3 -DMAX_ITERATIONS=1 -Wall -Wextra`
- ***Debug***: cmake will use this flags for compiling: `-D_DEBUG -DMAX_ITERATIONS=1 -Wall -Wextra`

The ***_DEBUG*** flag enable some part of the code used for debugging purpose.
The ***MAX_ITERATIONS*** flag indicate the number of iteration will be executed.
The compiled object can be found in the build dir in the root of project.
