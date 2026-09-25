#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-4
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=5:00:00
#SBATCH --mem=128G

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-1"
RESULTS="$ROOT_DIR/data/part4"
EXEC="$ROOT_DIR/main_O3"

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

make -C "$ROOT_DIR"

echo "running merge sort..."
for n in 1000000 10000000 100000000
do
    echo "n = $n"
    rm -f "$RESULTS/merge_sort_$n.csv"
    $EXEC merge_sort "$RESULTS/merge_sort_$n.csv" $n 10
    echo "finished!"
done

echo "finished merge sort!"