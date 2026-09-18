#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-4
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=20:00
#SBATCH --mem=128G

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-1"

EXEC="$ROOT_DIR/main"
RESULTS="$ROOT_DIR/results"

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

make clean -C "$ROOT_DIR"
make CXXFLAGS="-std=c++17 -O3" -C "$ROOT_DIR"

echo "running merge sort..."
for n in 1000000 10000000 100000000
do
    echo "n = $n"
    rm -f "$RESULTS/merge_sort.txt"
    $EXEC merge_sort $n 10 \
        >> "$RESULTS/merge_sort.txt"
    echo "finished!"
done
echo "finished merge sort!"