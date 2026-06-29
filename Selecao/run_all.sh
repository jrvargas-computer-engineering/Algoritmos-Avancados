#!/bin/bash
# run_all.sh — drives the full experiment suite (E1–E9) defined in
# experimental_analysis_plan.md and (optionally) the Python analysis.
#
#   ./run_all.sh           run every experiment, leave analysis to -a
#   ./run_all.sh -a        run every experiment, then analyze
#   ./run_all.sh -e E5 -a  run only E5, then analyze E5
#
# Data lands in data/benchmark_data.csv (schema in §1.7 of the plan).
# Analysis tables/figures land in analysis_output/.

set -u
mkdir -p data
CSV_FILE="data/benchmark_data.csv"
EXEC="./build/run_experiments"
RUN_ANALYSIS=false
TARGET_EXP="ALL"

# Per-cell wall-clock guard (§1.3, §E6, §E7). Degenerate cells (e.g. quickselect
# or naive-MoM on all-equal input) can blow up to ~quadratic; a timed-out run
# simply writes no CSV row, and the analysis tolerates the missing cell.
CELL_TIMEOUT=20

usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "  -h          Show this help message."
    echo "  -e <ID>     Run a specific experiment (e.g., E1, E3, E7). Default is ALL."
    echo "  -a          Run the Python analysis script after experiments finish."
    echo "  -n          Do not run the Python analysis."
    exit 0
}

while getopts "he:an" opt; do
    case ${opt} in
        h ) usage ;;
        e ) TARGET_EXP=$OPTARG ;;
        a ) RUN_ANALYSIS=true ;;
        n ) RUN_ANALYSIS=false ;;
        \? ) usage ;;
    esac
done

want() { [[ "$TARGET_EXP" == "ALL" || "$TARGET_EXP" == "$1" ]]; }

echo "Ensuring the experiment runner is built in release mode..."
make build/run_experiments || exit 1

HEADER="run_id,experiment_id,algorithm,n,k,k_frac,g,distribution,dist_param,instance_seed,algo_seed,trial_index,comparisons,accesses,recursion_depth,time_ns,batch_size,build,result_value,oracle_match,timestamp"

# Fresh CSV only for a full run; a single-experiment run appends so you can
# rebuild one experiment without discarding the rest.
if [ "$TARGET_EXP" == "ALL" ] || [ ! -f "$CSV_FILE" ]; then
    echo "$HEADER" > "$CSV_FILE"
fi

# =========================================================================
# PHASE A: PARALLEL COUNTING EXPERIMENTS (E1, E4, E7)
# Safe to run concurrently (operation counts are timing-independent).
# =========================================================================
TASK_LIST="tasks_phase_a.txt"
> $TASK_LIST

# E1: Comparisons & accesses vs n (UNI, k=median, g=5). Multiple seeds so the
# per-n mean/spread is meaningful for the log-log regression.
if want E1; then
    for n in 100 1000 5000 10000 50000 100000 500000; do
        for algo in naive quickselect mom; do
            for seed in $(seq 1 10); do
                echo "E1 $algo $n $((n/2)) 5 uni $seed" >> $TASK_LIST
            done
        done
    done
fi

# E4: k-dependency (UNI). Sweep k across the rank range; average over seeds.
if want E4; then
    for n in 10000 100000; do
        for k in 0 $((n/100)) $((n/4)) $((n/2)) $((3*n/4)) $((99*n/100)) $((n-1)); do
            for algo in naive quickselect mom; do
                for seed in $(seq 1 5); do
                    echo "E4 $algo $n $k 5 uni $seed" >> $TASK_LIST
                done
            done
        done
    done
fi

# E7: robustness / adversarial & duplicate stress. Small n sweep so the
# degradation curve is captured; the timeout guards the quadratic cells
# (quickselect & naive-MoM on alleq). nth_element included for E9.
if want E7; then
    for n in 500 1000 2000 4000; do
        for dist in uni gaussian alleq dup_2 dup_5 sorted_asc sorted_desc organpipe nearly_sorted; do
            for algo in naive quickselect mom nth_element; do
                for seed in $(seq 1 3); do
                    echo "E7 $algo $n $((n/2)) 5 $dist $seed" >> $TASK_LIST
                done
            done
        done
    done
fi

if [ -s $TASK_LIST ]; then
    echo "Executing Phase A (Parallel, ${CELL_TIMEOUT}s/cell, 6 cores)..."
    cat $TASK_LIST | xargs -n 7 -P 6 sh -c 'timeout '"$CELL_TIMEOUT"' $0 $1 $2 $3 $4 $5 $6 $7' "$EXEC"
fi
rm -f $TASK_LIST

# Sequential timing helper: R measured reps, each on its own instance/seed.
# (Each trial is a fresh process, so cross-trial cache warm-up is moot; the
# median over reps gives the robustness the plan's warm-up aimed for, §1.3.)
run_timed() {
    # args: exp algo n k g dist reps
    local exp=$1 algo=$2 n=$3 k=$4 g=$5 dist=$6 reps=$7
    for rep in $(seq 1 "$reps"); do
        timeout $CELL_TIMEOUT $EXEC "$exp" "$algo" "$n" "$k" "$g" "$dist" "$rep"
    done
}

# =========================================================================
# PHASE B: SEQUENTIAL TIMING EXPERIMENTS (E2/E8/E9, E3, E5, E6)
# Single-core to avoid cache thrashing between concurrent measurements.
# =========================================================================

# E2 / E8 / E9: wall-clock T(n) sweep (UNI, k=median, g=5). All four algorithms
# over the same instances; E8 reads these for crossover, E9 for nth_element.
if want E2 || want E8; then
    echo "Executing E2/E8/E9 (Time vs n)..."
    for n in 5000 10000 25000 50000 100000 250000; do
        for algo in naive quickselect mom nth_element; do
            run_timed E2 "$algo" "$n" "$((n/2))" 5 uni 30
        done
    done
fi

# E3: quickselect runtime distribution at fixed n (500 reps to expose the tail).
if want E3; then
    echo "Executing E3 (Quickselect Distribution)..."
    for n in 1000 10000 100000; do
        run_timed E3 quickselect "$n" "$((n/2))" 5 uni 500
    done
fi

# E5: median-of-medians group-size sweep (UNI, n fixed, k=median).
if want E5; then
    echo "Executing E5 (MoM Group Sizes)..."
    n=100000
    for g in 3 5 7 9 11 15 21; do
        run_timed E5 mom "$n" "$((n/2))" "$g" uni 30
    done
fi

# E6: the g=3 anomaly. Self-contained slope comparison: g in {3,5,7} across a
# WIDE n range (up to 1e6) -- the n*log(n) drift at g=3 is only visible over a
# wide span, since log(n) grows slowly. UNI/SORTED-ASC are distinct, so MoM
# avoids the duplicate blowup and g=3 stays cheap (~0.1s at n=1e6). MoM is
# deterministic per instance, so a handful of reps suffices for clean means.
if want E6; then
    echo "Executing E6 (g=3 Anomaly)..."
    for n in 1000 5000 10000 50000 100000 250000 500000 1000000; do
        for dist in uni sorted_asc; do
            for g in 3 5 7; do
                run_timed E6 mom "$n" "$((n/2))" "$g" "$dist" 8
            done
        done
    done
fi

echo "All experiments complete. Data saved in $CSV_FILE."

# Archive a timestamped copy so each run is dated and not overwritten by the
# next one. analyze.py keeps reading the canonical $CSV_FILE.
STAMP=$(date +%Y%m%dT%H%M%S)
ARCHIVE="data/benchmark_data_${STAMP}.csv"
cp "$CSV_FILE" "$ARCHIVE"
echo "Archived timestamped copy -> $ARCHIVE"

if [ "$RUN_ANALYSIS" = true ]; then
    echo "Starting analysis..."
    python3 analyze.py -e "$TARGET_EXP"
fi
