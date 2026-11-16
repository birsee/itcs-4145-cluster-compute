#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=a2-a
#SBATCH --exclusive
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=36
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=5:00:00
#SBATCH --mem=128G

# does the sequential part!

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-2"
RESULTS="$ROOT_DIR/data"
EXEC="$ROOT_DIR/main"

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

make -C "$ROOT_DIR"

# sequential tile sweep
echo "running tile sweep"
path="$RESULTS/1-1024-b0.csv"
rm -f "$path"
$EXEC "$path" tiled 1024 1024 1024 0 0
for b in 0 16 32 64 128 256
do
    echo "b = $b"
    path="$RESULTS/1-2048-b$b.csv"
    rm -f "$path"
    $EXEC "$path" tiled 2048 2048 2048 0 $b
    echo "finished!"
done

echo "finished all work!"