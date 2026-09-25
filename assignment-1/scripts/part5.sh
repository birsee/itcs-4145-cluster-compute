#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-name=pt-5
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=5:00:00
#SBATCH --mem=128G

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-1"
RESULTS="$ROOT_DIR/data/part5"
EXEC="$ROOT_DIR/main_O3"

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

make -C "$ROOT_DIR"

echo "running BFS..."

# generate the graphs
ER_GRAPH="$ROOT_DIR/graph_er.mtx"
RMAT_GRAPH="$ROOT_DIR/graph_rmat.mtx"
rm -f $ER_GRAPH
rm -f $RMAT_GRAPH
python3 "$ROOT_DIR/scripts/generate_data.py"

# run the actual operation
echo "er graph"
rm -f "$RESULTS/bfs_er.csv"
ER_SEARCH_RESULTS_PATH="$RESULTS/er/"
rm -f "$ER_SEARCH_RESULTS_PATH*"
$EXEC bfs "$RESULTS/bfs_er.csv" "$ER_SEARCH_RESULTS_PATH" "$ER_GRAPH" 16 10
echo "er graph finished!"

echo "rmat graph"
rm -f "$RESULTS/bfs_rmat.csv"
RMAT_SEARCH_RESULTS_PATH="$RESULTS/rmat/"
rm -f "$RMAT_SEARCH_RESULTS_PATH*"
$EXEC bfs "$RESULTS/bfs_rmat.csv" "$ER_SEARCH_RESULTS_PATH" "$RMAT_GRAPH" 16 10
echo "rmat graph finished!"

echo "finished part 5!"