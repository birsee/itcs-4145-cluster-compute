#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=itsc-4145-assignment-1
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=1:00:00
#SBATCH --mem=128G

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-1"

EXEC="$ROOT_DIR/main"
RESULTS="$ROOT_DIR/results"

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

echo "running experiments!"

echo "running array max..."
for n in 1000000 10000000 100000000
do
    echo "Array size: $n"
    $EXEC array_max $n 10 \
        >> "$RESULTS/array_max.txt"
done

echo "running matrix multiplication..."
for n in 256 512 1024
do
    echo "Matrix size: $n"
    $EXEC matmul $n 10 \
        >> "$RESULTS/matmul.txt"
done

echo "running merge sort..."
for n in 1000000 10000000 100000000
do
    $EXEC merge_sort $n 10 \
        >> "$RESULTS/merge_sort.txt"
done

echo "running BFS..."
# generate the graphs if they don't exist
ER_GRAPH="$ROOT_DIR/graph_er.mtx"
RMAT_GRAPH="$ROOT_DIR/graph_rmat.mtx"
if [ ! -f "$ER_GRAPH" ] || [ ! -f "$RMAT_GRAPH" ]; then
    python3 "$ROOT_DIR/scripts/generate_data.py"
done
# run the actual operation
$EXEC bfs ER_GRAPH 16 \
    >> "$RESULTS/bfs_er.txt"
$EXEC bfs RMAT_GRAPH 16 \
    >> "$RESULTS/bfs_rmat.txt"

echo "running prefix sum..."
for opt in O0 O2 O3 do
    make clean
    make CXXFLAGS="-std=c++17 -$opt"

    for n in 1000000 10000000 100000000 do
        echo "n=$n optimization=-$opt"
        ./main prefix_sum $n 10 \
            >> results/prefix_sum_$opt.txt
    done
done

echo "finished!"