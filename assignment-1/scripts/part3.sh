#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=itsc-4145-assignment-1-pt-3
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

echo "running matrix multiplication..."
for n in 256 512 1024
do
    echo "n = $n"
    $EXEC matmul $n $n $n 10 \
        >> "$RESULTS/matmul.txt"
    echo "finished!"
done

echo "m = 256 | k = 300 | n = 512"
$EXEC matmul 256 300 512 10 \
    >> "$RESULTS/matmul_irregular.txt"
echo "finished!"

echo "finished matrix multiplication!"