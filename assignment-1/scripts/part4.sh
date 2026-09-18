#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=itsc-4145-assignment-1-pt-4
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

echo "running merge sort..."
for n in 1000000 10000000 100000000
do
    echo "n = $n"
    $EXEC merge_sort $n 10 \
        >> "$RESULTS/merge_sort.txt"
done