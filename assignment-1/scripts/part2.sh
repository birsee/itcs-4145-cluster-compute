#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-2
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

echo "running prefix sum..."
for opt in O0 O2 O3
do
    make clean -C "$ROOT_DIR"
    make CXXFLAGS="-std=c++17 -$opt" -C "$ROOT_DIR"

    rm -f "$RESULTS/prefix_sum_$opt.txt"
    for n in 1000000 10000000 100000000
    do
        echo "n = $n | optimization=-$opt"
        $EXEC prefix_sum $n 10 \
            >> "$RESULTS/prefix_sum_$opt.txt"
        echo "finished!"
    done
done
echo "finished prefix sum!"