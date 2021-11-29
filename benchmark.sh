#!/bin/bash

for p in {1..8}
do
  OUTPUT=$(mpirun --np $p ./build/req_parallel ./build/vector_float_1G.dat | grep "ParallelREQ")
  echo $OUTPUT
done