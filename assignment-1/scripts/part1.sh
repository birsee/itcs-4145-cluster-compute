#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-1
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

echo "running array max..."
for n in 1000000 10000000 100000000
do
    echo "n = $n"
    $EXEC array_max $n 10 \
        >> "$RESULTS/array_max.txt"
    echo "finished!"
done
echo "finished array max!"