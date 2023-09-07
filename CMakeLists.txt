cmake_minimum_required(VERSION 3.16)
project(ParallelREQ CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -DMAX_ITERATIONS=1 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -DMAX_ITERATIONS=1 -Wall -Wextra")
set(CMAKE_CXX_COMPILER mpiicpc)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH} "lib/req/include" "lib/common/include")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

add_executable(req_sequential src/req_sequential.cpp)
add_executable(bin_generator src/bin_generator.cpp)
add_executable(intro_select src/intro_select.cpp)
add_executable(req_parallel src/req_parallel.cpp)
target_link_libraries(req_parallel_v4 ${MPI_LIBRARIES})