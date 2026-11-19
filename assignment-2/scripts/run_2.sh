#!/bin/bash

#SBATCH --partition=Centaurus
#SBATCH --job-path=a2-a
#SBATCH --exclusive
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=36
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=5:00:00
#SBATCH --mem=128G

export OMP_PROC_BIND=true
export OMP_PLACES=true

set -e

ROOT_DIR="$SLURM_SUBMIT_DIR/assignment-2"
RESULTS="$ROOT_DIR/data"
EXEC="$ROOT_DIR/main"
BEST_B=64

cd "$ROOT_DIR"
mkdir -p "$RESULTS"

make -C "$ROOT_DIR"

for type in "pthread" "omp"
do
    # row test
    echo "running row $type"
    
    for b in 0 $BEST_B
    do
        for sz in 1024 2048
        do
            for p in 1 2 4 8 12 18 36
            do
                echo "b = $b, size = $sz, p = $p"
                path="$RESULTS/$type-row-b$b-$sz-p$p.csv"
                rm -f "$path"
                $EXEC "$path" $type-row $sz $sz $sz 0 $b $p
                echo "finished!"
            done
        done
    done
    
    # block tests
    echo "running block $type"
    for sz in 1024 2048
    do
        for p in 2 4 8 12 18 36
        do
            pr=1
            for ((d = 1; d * d <= p; d++)); do
                if (( p % d == 0 )); then pr=$d; fi
            done
            pc=$((p / pr))

            path="$RESULTS/$type-block-b$BEST_B-$sz-pr$pr-pc$pc.csv"
            rm -f "$path"
            $EXEC "$path" $type-block $sz $sz $sz 0 $BEST_B $pr $pc
        done
    done

    echo "block column test"
    path="$RESULTS/$type-block-b$BEST_B-2048-pr1-pc36.csv"
    rm -f "$path"
    $EXEC "$path" $type-block 2048 2048 2048 0 $BEST_B 1 36

    # non divisible test
    # don't actually need the results here but generating them means it works correctly
    echo "running non-divisible $type"
    path="$RESULTS/$type-nondiv.csv"
    rm -f "$path"
    $EXEC "$path" $type-block 1000 1024 1536 0 $BEST_B 6 6
done

echo "finished all work!"