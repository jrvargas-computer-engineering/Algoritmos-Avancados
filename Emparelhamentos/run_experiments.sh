#!/bin/bash

# =============================================================================
# SYSTEM CONSTANTS & VALIDATION
# =============================================================================
BENCHMARK_BIN="./build/benchmark_runner"
RAW_DIR="raw"
COMBINED_DIR="data/combined"

# Max concurrent benchmark processes for count-based (timing-insensitive) tests.
# Timing tests always run sequentially regardless of this value.
#
# The default is all cores, but capped for memory: each process holds an O(n^2)
# weight matrix, measured at ~200 MB for the largest parallel size (U1, n=5000).
# We budget 256 MB/process and keep peak usage under half of currently-available
# RAM, so the machine stays responsive on 16 GB even with other apps open.
# Override explicitly with: JOBS=4 ./run_experiments.sh ...
default_jobs() {
    local cores cap mem_avail_mb
    cores=$(nproc)
    mem_avail_mb=$(( $(awk '/MemAvailable/ {print $2}' /proc/meminfo) / 1024 ))
    cap=$(( mem_avail_mb / 2 / 256 ))   # half of available RAM / 256 MB per process
    [ "$cap" -lt 1 ] && cap=1
    [ "$cap" -lt "$cores" ] && cores=$cap
    echo "$cores"
}
JOBS="${JOBS:-$(default_jobs)}"

if [ ! -f "$BENCHMARK_BIN" ]; then
    echo "[FATAL] Benchmark binary absent. Execute 'make benchmark_runner' first."
    exit 1
fi

# =============================================================================
# CLI ROUTING & PARSING
# =============================================================================
SMOKE_MODE=0
RUN_ALL=0
NO_ANALYSIS=0
TARGETS=()

print_usage() {
    echo "Usage: ./run_experiments.sh [OPTIONS]"
    echo "Options:"
    echo "  --smoke       Overrides all repetition budgets to 1 for end-to-end I/O validation."
    echo "  --all         Executes all 12 tests."
    echo "  --oracles     Executes Group 0 (U0, W0) Correctness Gates."
    echo "  --unweighted  Executes Group 1 (U1-U4) Unweighted Baselines."
    echo "  --weighted    Executes Group 2 (W1-W4) Weighted Baselines."
    echo "  --master      Executes Group 3 (C1, C2) Master Scalability Tables."
    echo "  --test <ID>   Executes a specific isolated test (e.g., --test U3)."
    echo "  --no-analysis Generates raw data only; skips figures (run 'make analyze' later)."
    echo ""
    echo "Count-based tests (U0,W0,U1,U2,W1,W2) run in parallel across \$JOBS cores;"
    echo "timing tests (U3,U4,W3,W4,C1,C2) run sequentially to keep wall-clock valid."
    echo "Set JOBS to cap parallelism, e.g. JOBS=4 ./run_experiments.sh --all"
    exit 1
}

if [ "$#" -eq 0 ]; then print_usage; fi

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --smoke) SMOKE_MODE=1; shift ;;
        --all) RUN_ALL=1; shift ;;
        --no-analysis) NO_ANALYSIS=1; shift ;;
        --oracles) TARGETS+=("u0" "w0"); shift ;;
        --unweighted) TARGETS+=("u1" "u2" "u3" "u4"); shift ;;
        --weighted) TARGETS+=("w1" "w2" "w3" "w4"); shift ;;
        --master) TARGETS+=("c1" "c2"); shift ;;
        --test) TARGETS+=("$(echo "$2" | tr '[:upper:]' '[:lower:]')"); shift 2 ;;
        -h|--help) print_usage ;;
        *) echo "[FATAL] Unknown parameter: $1"; print_usage ;;
    esac
done

if [ "$RUN_ALL" -eq 1 ]; then
    TARGETS=("u0" "w0" "u1" "u2" "u3" "u4" "w1" "w2" "w3" "w4" "c1" "c2")
fi

# Deduplicate targets
TARGETS=($(echo "${TARGETS[@]}" | tr ' ' '\n' | sort -u | tr '\n' ' '))

# Ensure output directories exist (the benchmark binary fails silently if raw/
# is missing). Safe to run even when the directories already exist.
mkdir -p "$RAW_DIR" "$COMBINED_DIR"

# =============================================================================
# PARALLEL EXECUTION HELPERS
# =============================================================================
# Tests whose report figures depend on wall_clock_ms must run one at a time so
# that timing is not distorted by CPU, cache, and memory-bandwidth contention.
# Every other test measures deterministic counts and is safe to run in parallel.
is_timing_test() {
    case "$1" in
        U3|U4|W3|W4|C1|C2) return 0 ;;
        *) return 1 ;;
    esac
}

# Dispatches a single benchmark invocation. Count-based tests are backgrounded
# and throttled to $JOBS concurrent processes; timing tests run in the
# foreground so each is measured on an otherwise idle machine.
launch() {
    if is_timing_test "$1"; then
        "$BENCHMARK_BIN" "$@"
    else
        while [ "$(jobs -r -p | wc -l)" -ge "$JOBS" ]; do wait -n; done
        "$BENCHMARK_BIN" "$@" &
    fi
}

# =============================================================================
# DATA AGGREGATION PROTOCOL
# =============================================================================
aggregate_test_data() {
    local test_id=$1
    local output_file="${COMBINED_DIR}/${test_id}.csv"

    # Barrier: wait for any backgrounded count-test jobs to finish before
    # concatenating their raw files.
    wait

    echo "      [INFO] Aggregating raw telemetry into ${output_file}..."
    echo "test_id,n,alpha,rep,algorithm,iteration,weight_regime,metric_name,metric_value" > "$output_file"
    find "$RAW_DIR" -name "${test_id}_*.csv" -exec tail -q -n +2 {} + >> "$output_file"
}

# =============================================================================
# GROUP 0: ORACLE CORRECTNESS GATES
# =============================================================================
execute_u0() {
    local test_id="U0"
    local reps=$(( SMOKE_MODE ? 1 : 50 ))
    local weight_regime="na"
    local alphas=(1.0 1.5 2.0)
    local n_vals=(2 3 4 5 6)
    
    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

execute_w0() {
    local test_id="W0"
    local reps=$(( SMOKE_MODE ? 1 : 100 ))
    local weight_regime="mixed"
    local alpha=1.5
    local n_vals=(2 3 4)
    
    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for (( r=0; r<reps; r++ )); do
            launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
        done
    done
    aggregate_test_data "$test_id"
}

# =============================================================================
# GROUP 1: UNWEIGHTED MATCHING ENGINES
# =============================================================================
execute_u1() {
    local test_id="U1"
    local reps=$(( SMOKE_MODE ? 1 : 10 ))
    local weight_regime="na"
    local alphas=(1.0 1.5 2.0)
    local n_vals=(50 100 200 500 1000 2000 5000)

    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

execute_u2() {
    local test_id="U2"
    local reps=$(( SMOKE_MODE ? 1 : 5 ))
    local weight_regime="na"
    local alphas=(1.0 1.5 2.0)
    local n_vals=(200 500 1000 2000)

    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

execute_u3() {
    local test_id="U3"
    local reps=$(( SMOKE_MODE ? 1 : 20 ))
    local weight_regime="na"
    local alphas=(1.0 1.25 1.5 1.75 2.0)
    
    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for (( n=100; n<=10000; n*=2 )); do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

execute_u4() {
    local test_id="U4"
    local reps=$(( SMOKE_MODE ? 1 : 15 ))
    local weight_regime="na"
    local alphas=(1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9 2.0)
    local n_vals=(500 1000 2000)

    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

# =============================================================================
# GROUP 2: WEIGHTED MATCHING ENGINES
# =============================================================================
execute_w1() {
    local test_id="W1"
    local reps=$(( SMOKE_MODE ? 1 : 10 ))
    local weight_regime="mixed"
    local alphas=(1.0 1.5 2.0)
    local n_vals=(50 100 200 500 1000)

    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

execute_w2() {
    local test_id="W2"
    local reps=$(( SMOKE_MODE ? 1 : 10 ))
    local weight_regime="mixed"
    local alphas=(1.0 1.5 2.0)
    local n_vals=(50 100 200 500 1000 2000)

    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

execute_w3() {
    local test_id="W3"
    local reps=$(( SMOKE_MODE ? 1 : 20 ))
    local weight_regime="mixed"
    local alphas=(1.0 1.25 1.5 1.75 2.0)
    
    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for (( n=50; n<=2000; n*=2 )); do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

execute_w4() {
    local test_id="W4"
    local reps=$(( SMOKE_MODE ? 1 : 20 ))
    local regimes=("mixed" "negative" "positive")
    local alpha=1.5
    local n_vals=(100 500 1000)

    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for weight_regime in "${regimes[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

# =============================================================================
# GROUP 3: HEAD-TO-HEAD MASTER TABLES
# =============================================================================
execute_c1() {
    local test_id="C1"
    local reps=$(( SMOKE_MODE ? 1 : 30 ))
    local weight_regime="na"
    local alphas=(1.0 1.5 2.0)
    local n_vals=(100 500 1000 2000 5000 10000)

    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

execute_c2() {
    local test_id="C2"
    local reps=$(( SMOKE_MODE ? 1 : 30 ))
    local weight_regime="mixed"
    local alphas=(1.0 1.5 2.0)
    local n_vals=(50 100 200 500 1000 2000)

    echo "[EXEC] Triggering ${test_id} Matrix Sweep (${reps} reps/cell)"
    for n in "${n_vals[@]}"; do
        for alpha in "${alphas[@]}"; do
            for (( r=0; r<reps; r++ )); do
                launch "$test_id" "$n" "$alpha" "$r" "$weight_regime"
            done
        done
    done
    aggregate_test_data "$test_id"
}

# =============================================================================
# MASTER EXECUTION ROUTER
# =============================================================================
echo "=================================================="
echo "INITIALIZING BIPARTITE BENCHMARK PIPELINE"
echo "Parallelism: ${JOBS} core(s) for count tests; timing tests run sequentially."
if [ "$SMOKE_MODE" -eq 1 ]; then
    echo "[WARNING] SMOKE MODE ACTIVE: All repetition budgets forced to 1."
fi
echo "=================================================="

for target in "${TARGETS[@]}"; do
    case $target in
        u0) execute_u0 ;;
        w0) execute_w0 ;;
        u1) execute_u1 ;;
        u2) execute_u2 ;;
        u3) execute_u3 ;;
        u4) execute_u4 ;;
        w1) execute_w1 ;;
        w2) execute_w2 ;;
        w3) execute_w3 ;;
        w4) execute_w4 ;;
        c1) execute_c1 ;;
        c2) execute_c2 ;;
    esac
done

if [ "$NO_ANALYSIS" -eq 1 ]; then
    echo "=================================================="
    echo "DATA GENERATION COMPLETE (analysis skipped)"
    echo "Run 'make analyze' to produce tables and figures."
    echo "=================================================="
    exit 0
fi

echo "=================================================="
echo "INITIATING DATA REDUCTION & CURVE FITTING"
echo "=================================================="

if command -v python3 &>/dev/null; then
    python3 -m analysis.run_all --data-dir "$COMBINED_DIR" --out-dir "analysis_output"
else
    echo "[WARNING] python3 not found. Raw data generated, but analytical tables skipped."
fi

echo "=================================================="
echo "PIPELINE TERMINATED SUCCESSFULLY"
echo "=================================================="