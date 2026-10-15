#!/bin/bash
#SBATCH --job-name=inclass-pi
#SBATCH --partition=Centaurus
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=36
#SBATCH --exclusive
#SBATCH --time=00:20:00
#SBATCH --output=pi-%j.log

# ITCS 4145 in-class exercise, pre-class check: runs every step of the
# handout with the instructor's solution (pi.c) and the step 2 program
# (pi_shared.c). The log holds the numbers the students should see.

set -e
echo "node: $(hostname)"
lscpu | grep -E 'Model name|^CPU\(s\)|Socket'
gcc --version | head -1
gcc -O2 -Wall -pthread pi.c        -o pi
gcc -O2 -Wall -pthread pi_shared.c -o pi_shared

echo "== step 1: sequential"
for n in 10000000 100000000 1000000000; do ./pi $n 0; done

echo "== step 2: one shared counter, n = 1e8, p = 8, three runs"
for i in 1 2 3; do ./pi_shared 100000000 8; done

echo "== step 3: own slot per thread, n = 1e8, p = 8, three runs; then p = 1"
for i in 1 2 3; do ./pi 100000000 8; done
./pi 100000000 1

echo "== step 4: timing, n = 1e9"
for p in 1 2 4 8 18 36; do ./pi 1000000000 $p; done
echo "done"
