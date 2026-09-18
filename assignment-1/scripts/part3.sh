#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-3
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

echo "running matrix multiplication..."
for n in 256 512 1024
do
    echo "n = $n"
    rm -f "$RESULTS/matmul.txt"
    $EXEC matmul $n $n $n 10 \
        >> "$RESULTS/matmul.txt"
    echo "finished!"
done

echo "m = 256 | k = 300 | n = 512"
rm -f "$RESULTS/matmul_irregular.txt"
$EXEC matmul 256 300 512 10 \
    >> "$RESULTS/matmul_irregular.txt"
echo "finished!"

echo "finished matrix multiplication!"