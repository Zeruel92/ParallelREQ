# ParallelREQ

ParallelREQ is an MPI parallel implementation of the REQ algorithm for computing quantiles in streaming of data

The sequential code used can be found at: https://datasketches.apache.org/docs/Community/Downloads.html

## Project Structure

### bin_generator

`bin_generator` is a utility to generate binary file with `n` numbers in uniform distribution `[0,1)`

Usage: `./bin_generator <n> <outputfile>`

The output file has as first element the number of total elements stored, and following all the others elements.

### req_sequential

`req_sequential` uses the apache datasketches library to compute quantiles in sequential

### req_parallel

`req_parallel` uses the `Apache datasketches` library to compute quantiles in parallel.
Usage: `mpirun --np <number of processes> ./req_parallel <fileinput>`
Each process read the file in input to retrieve the total number of elements `n`.
After that each process compute the own block of data and read it from file.

Once every process has finished read the data, using the `Apache datasketches` they compute the intermediate summary.

Finally, using a user defined reduction the intermediate summaries are merged into one.

### benchmark.sh

Is a simple script that execute req_parallel with a number of processes ranging from 1 to 8,
in order to take some performance measure

## Compiling