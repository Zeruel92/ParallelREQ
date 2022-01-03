#!/bin/bash

for p in {1..4}
do
  OUTPUT=$(mpirun --np $p ./build/req_parallel_v4 ./build/vector_float_1G.dat ./build/ground_truth_1G.dat 0 | grep "ParallelREQ")
  echo $OUTPUT
done