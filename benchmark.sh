#!/bin/bash

for p in {1..4}
do
  OUTPUT=$(mpirun --np $p ./build/req_parallel ./build/vector_float_500M.dat ./build/ground_truth_500M.dat 0 )
  echo $OUTPUT
  echo "------ 500M benchmark --------" >> benchmark.txt
  echo $OUTPUT >> benchmark.txt
  echo "------------------------------" >> benchmark.txt
done

for p in {1..4}
do
  OUTPUT=$(mpirun --np $p ./build/req_parallel ./build/vector_float_1G.dat ./build/ground_truth_1G.dat 0)
  echo $OUTPUT
  echo "------ 1G benchmark --------" >> benchmark.txt
  echo $OUTPUT >> benchmark.txt
  echo "------------------------------" >> benchmark.txt
done

for p in {1..4}
do
  OUTPUT=$(mpirun --np $p ./build/req_parallel ./build/vector_float_2G.dat ./build/ground_truth_2G.dat 0 )
  echo $OUTPUT
  echo "------ 2G benchmark --------" >> benchmark.txt
  echo $OUTPUT >> benchmark.txt
  echo "------------------------------" >> benchmark.txt
done