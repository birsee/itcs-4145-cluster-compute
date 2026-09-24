#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-5
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=5:00:00
#SBATCH --mem=128G

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-1"
RESULTS="$ROOT_DIR/data"
EXEC="$ROOT_DIR/main_O3"

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

make -C "$ROOT_DIR"

echo "running BFS..."

# generate the graphs if they don't exist
ER_GRAPH="$ROOT_DIR/graph_er.mtx"
RMAT_GRAPH="$ROOT_DIR/graph_rmat.mtx"
if [ ! -f "$ER_GRAPH" ] || [ ! -f "$RMAT_GRAPH" ]; then
    python3 "$ROOT_DIR/scripts/generate_data.py"
fi

# run the actual operation
echo "er graph"
rm -f "$RESULTS/bfs_er.csv"
$EXEC bfs "$RESULTS/bfs_er.csv" "$ER_GRAPH" 16 10
echo "er graph finished!"

echo "rmat graph"
rm -f "$RESULTS/bfs_rmat.csv"
$EXEC bfs "$RESULTS/bfs_rmat.csv" "$RMAT_GRAPH" 16 10
echo "rmat graph finished!"

echo "finished BFS!"