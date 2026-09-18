#!/bin/bash

set -e

lscpu | grep -i cache > cpu_cache_info.txt
cat cpu_cache_info.txt

if [ ! -f "erdos_renyi.mtx" ]; then
    echo "generating graph data"
    python3 generate_graphs.py
fi

# compile
echo "compiling with -O3..."
make clean
make CXXFLAGS="-std=c++17 -O3 -Wall -Wextra -Iinclude"

mkdir -p results

echo "PT 1:"
for n in 1000000 10000000 100000000; do
    ./final array_max $n 10 >> results/array_max.txt
done

# recomp stuff for part 2
echo "PT 2:"
for OPT in "-O0" "-O2" "-O3"; do
    echo "compiling with $OPT..."
    make clean
    make CXXFLAGS="-std=c++17 $OPT -Wall -Wextra -Iinclude"
    
    echo "results for $OPT:" >> results/prefix_sum.txt
    for n in 1000000 10000000 100000000; do
        ./final prefix_sum $n 10 >> results/prefix_sum.txt
    done
done

echo "PT 3:"
for n in 256 512 1024; do
    ./final matmul $n 10 >> results/matmul.txt
done
./final matmul_irregular 512 1024 256 10 >> results/matmul.txt

echo "PT 4:"
for n in 1000000 10000000 100000000; do
    ./final merge_sort $n 10 >> results/merge_sort.txt
done

echo "PT 5:"
./final bfs erdos_renyi.mtx 16 >> results/bfs_er.txt
./final bfs rmat.mtx 16 >> results/bfs_rmat.txt

echo "experiments complete! data saved to results directory."