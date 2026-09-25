#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-2
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=5:00:00
#SBATCH --mem=128G

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-1"
RESULTS="$ROOT_DIR/data"

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

echo "running prefix sum..."
for opt in O0 O2 O3
do
    make OPT=-$opt
    EXEC="$ROOT_DIR/main_$opt"
    for n in 1000000 10000000 100000000
    do
        echo "n = $n | optimization=-$opt"
        rm -f "$RESULTS/prefix_sum_$n$opt.csv"
        $EXEC prefix_sum "$RESULTS/prefix_sum_$n$opt.csv" $n 10 $opt
        echo "finished!"
    done
done

echo "finished prefix sum!"