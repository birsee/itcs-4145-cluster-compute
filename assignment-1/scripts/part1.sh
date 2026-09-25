#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-1
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=5:00:00
#SBATCH --mem=128G

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-1"
RESULTS="$ROOT_DIR/data/part1"
EXEC="$ROOT_DIR/main_O3"

cd "$ROOT_DIR"
mkdir -p "$RESULTS/max"
mkdir -p "$RESULTS/bandwidth"

make -C "$ROOT_DIR"

echo "running array max..."
for n in 1000000 10000000 100000000
do
    echo "n = $n"
    rm -f "$RESULTS/max/array_max_$n.csv"
    rm -f "$RESULTS/bandwidth/array_bandwidth_$n.csv"
    $EXEC array_max "$RESULTS/max/array_max_$n.csv" "$RESULTS/bandwidth/array_bandwidth_$n.csv" $n 10
    echo "finished!"
done

echo "finished array max!"