#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-3
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=5:00:00
#SBATCH --mem=128G

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-1"
RESULTS="$ROOT_DIR/data/part3"
EXEC="$ROOT_DIR/main_O3"

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

make -C "$ROOT_DIR"

echo "running matrix multiplication..."
for n in 256 512 1024
do
    echo "n = $n"
    rm -f "$RESULTS/matmul_$n.csv"
    $EXEC matmul "$RESULTS/matmul_$n.csv" $n $n $n 10
    echo "finished!"
done

echo "m = 256 | k = 300 | n = 512"
rm -f "$RESULTS/matmul_irregular.csv"
$EXEC matmul "$RESULTS/matmul_irregular.csv" 256 300 512 10
echo "finished!"

echo "finished matrix multiplication!"