# Bipartite Matching — Experimental Analysis

Empirical evaluation of four maximum-weight bipartite matching algorithms:
**SimpleAugmentingMatcher**, **HopcroftKarpMatcher**, and two variants of the
**HungarianMatcher** using Bellman-Ford (**BF**) and Johnson/Dijkstra (**JD**)
as shortest-path strategies.

---

## Requirements

| Tool | Version tested |
|---|---|
| g++ | 13.3.0 |
| make | GNU Make 4.x |
| Python | 3.10+ |
| Python packages | `pandas`, `numpy`, `matplotlib`, `scipy` |

Install Python dependencies with:
```bash
pip install pandas numpy matplotlib scipy
```

---

## Build

```bash
# Production binary (reads graph from stdin, writes matching value to stdout)
make all

# Benchmark binary (writes instrumented CSV telemetry to raw/)
make benchmark_runner

# Unit test suite (four verification steps)
make test

# Remove all build artefacts and generated data
make clean
```

Build output goes to `build/` (production and test objects) and `build/bench/`
(benchmark objects compiled with `-DBENCHMARK_MODE`). Both directories are
created automatically by the build system.

---

## Running the experiments

The shell script `run_experiments.sh` drives the full pipeline: it calls the
benchmark binary, aggregates the raw CSV files, and then invokes the Python
analysis suite.

```bash
# Run everything (all 12 tests, full repetition budgets — takes several hours)
./run_experiments.sh --all

# Run only correctness gates (fast, ~seconds)
./run_experiments.sh --oracles

# Run a specific group
./run_experiments.sh --unweighted   # U1–U4
./run_experiments.sh --weighted     # W1–W4
./run_experiments.sh --master       # C1, C2

# Run a single test
./run_experiments.sh --test U3

# Smoke mode: forces 1 repetition per cell — useful for end-to-end I/O checks
./run_experiments.sh --smoke --all
```

The benchmark binary must exist before running the script. If it does not,
build it with `make benchmark_runner` first.

### Parallelism and memory

Count-based tests (U0, W0, U1, U2, W1, W2) measure deterministic quantities and
run **in parallel** across CPU cores. Timing tests (U3, U4, W3, W4, C1, C2)
measure wall-clock time and run **sequentially**, so their results are not
distorted by CPU, cache, or memory-bandwidth contention.

By default the script uses all cores, but caps the count automatically so that
peak memory stays under half of the available RAM (each process holds an O(n²)
weight matrix, ~200 MB at the largest parallel size). The chosen value is
printed at startup. Override it explicitly when needed:

```bash
# Cap parallelism to 4 processes (e.g. when using the machine for other work)
JOBS=4 ./run_experiments.sh --all
```

### Staged execution (data first, figures later)

The long timing runs dominate the total time. To generate the raw data without
waiting on the analysis step, pass `--no-analysis`, then render tables and
figures separately once the data is ready:

```bash
# Stage 1: generate all data, skip analysis
./run_experiments.sh --all --no-analysis

# Stage 2: produce tables and figures from data/combined/
make analyze
```

`make analyze` runs the full analysis suite, so it requires data for every test
to be present; run it only after a complete `--all` data generation.

---

## Using the production binary

The production binary reads a bipartite graph from **stdin** and writes the
maximum matching value to **stdout**:

```bash
echo "3
0 1 10
0 2 5
1 2 8" | ./build/matcher
```

The input format is defined in `src/include/IOHandler.h`.

---

## Directory layout

```
src/
  *.cpp / include/*.h     — algorithm implementations and interfaces
  tests/                  — unit test sources
Makefile                  — build system
run_experiments.sh        — experiment orchestration script
analysis/
  run_all.py              — analysis entry point (called by the shell script)
  plots/                  — one module per test (u1.py … c2.py)
  aggregate.py            — data aggregation utilities
  config.py               — shared plot configuration
  fit.py                  — power-law curve fitting
  load.py                 — CSV loading helpers
experiment_plan.md        — detailed specification of all 12 tests (goals,
                            n/α ranges, repetition budgets, CSV schema)
analysis_plan.md          — specification of every analysis output (plots,
                            tables, correctness checks, fitting conventions)
build/                    — compiled objects and binaries (generated)
build/bench/              — benchmark objects compiled with -DBENCHMARK_MODE
raw/                      — per-run CSV files written by the benchmark binary
data/combined/            — aggregated CSVs, one file per test (U1.csv … C2.csv)
analysis_output/          — derived tables and figures produced by the analysis
figures/                  — publication-quality plots (referenced by the report)
```

The last four directories are created by `make scaffold` / `make all` and are
not tracked by git.

---

## Reproducing the results

```bash
# 1. Clean state
make clean

# 2. Build both binaries
make all
make benchmark_runner

# 3. Run the full experiment suite
./run_experiments.sh --all

# 4. Figures and tables are written to analysis_output/figures/
```

To verify correctness before running long experiments:
```bash
make test                                      # unit tests
./run_experiments.sh --smoke --oracles         # oracle correctness gates
```

---

## Plan files

**`experiment_plan.md`** — the authoritative specification for all 12 tests.
Covers the algorithmic level of observation (main-loop iterations, inner-search
cost, wall-clock time), the parameters varied in each test (n, α, weight
regime), repetition budgets, the CSV column schema, and the definition of every
metric name emitted by the benchmark binary.

**`analysis_plan.md`** — the specification for the analysis pipeline. Describes
every plot and derived table produced by the Python scripts, the aggregation
conventions (median, p90 bands), power-law fitting procedure, and the
correctness checks that must pass before performance analysis proceeds.
