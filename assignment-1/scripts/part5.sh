#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-5
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

echo "running BFS..."

# generate the graphs if they don't exist
ER_GRAPH="$ROOT_DIR/graph_er.mtx"
RMAT_GRAPH="$ROOT_DIR/graph_rmat.mtx"
if [ ! -f "$ER_GRAPH" ] || [ ! -f "$RMAT_GRAPH" ]; then
    python3 "$ROOT_DIR/scripts/generate_data.py"
fi

# run the actual operation
echo "er graph"
rm -f "$RESULTS/bfs_er.txt"
$EXEC bfs "$ER_GRAPH" 16 > "$RESULTS/bfs_er.txt"
echo "er graph finished!"

echo "rmat graph"
rm -f "$RESULTS/bfs_er.txt"
$EXEC bfs "$RMAT_GRAPH" 16 > "$RESULTS/bfs_rmat.txt"
echo "rmat graph finished!"

echo "finished BFS!"