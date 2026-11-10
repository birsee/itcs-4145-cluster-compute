#!/bin/bash
# ITCS 4145, in-class exercise: the degree sum, both instances.
# Submit from this folder:  sbatch run_degree.slurm  ->  degree-<jobid>.out
# For the C++ version, change PROG below to degree_cpp.
#
#SBATCH --job-name=degree
#SBATCH --partition=Centaurus
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=00:10:00
#SBATCH --output=degree-%j.out

PROG=degree

set -u
echo "node: $(hostname)   date: $(date)"
make $PROG || exit 1

echo
./$PROG 1
echo
./$PROG 2
