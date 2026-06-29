# Assignment 4: Selection of the $k$-th Element

## Introduction
This project implements and empirically compares three algorithms for finding the $k$-th smallest element in an unsorted array:
*   **Naive Selection:** Copies the array, sorts it using `std::sort`, and accesses the $k$-th index. Serves as our baseline and correctness oracle.
*   **Randomized Quickselect:** Uses the Lomuto partition scheme with a randomly chosen pivot. Expected time complexity is $O(n)$.
*   **Median-of-Medians:** A deterministic selection algorithm that guarantees $O(n)$ worst-case performance by partitioning the array into groups of size $g$ and recursively finding the median of medians to use as a pivot.

The goal of this project is to test these algorithms theoretically and empirically, measuring exact operation counts (comparisons and accesses), wall-clock execution time, distribution of performance, and the practical crossover points where one algorithm outperforms another.

---

## Prerequisites
To build and run this project, you need:
*   **C++17 Compiler:** `g++` (with AddressSanitizer support for testing).
*   **Make:** GNU Make for the build system.
*   **Bash:** To execute the experiment orchestration script (supports `xargs` for multicore execution).
*   **Python 3:** For data analysis and graph generation.
*   **Python Libraries:** `pandas`, `numpy`, `scipy`, `matplotlib` (install via `pip install pandas numpy scipy matplotlib`).

---

## Makefile Options

The build system places all generated executables in the `build/` directory. It uses two distinct compilation profiles: a debug profile with AddressSanitizer (`-fsanitize=address`) for the TDD suite and main CLI, and a highly optimized release profile (`-O3 -DNDEBUG`) for the experiment runner to ensure accurate timing.

*   `make` or `make all`
    Compiles the main application CLI, the test suite, and the experiment runner.
*   `make test`
    Compiles the test suite (if changes were made) and immediately executes it. This checks base correctness and catches memory leaks or out-of-bounds accesses.
*   `make clean`
    Removes the `build/` directory and all compiled executables. **Note:** This will *not* delete your generated data (`benchmark_data.csv`) or any generated plots (`.png` files).

---

## Step-by-Step Guide: Running Experiments

The experimental pipeline is managed by `run_all.sh`. It automatically divides the work into two phases tailored to your CPU (Intel Core Ultra 5, 8 cores):
1.  **Phase A (Parallel):** Counting experiments (e.g., comparisons) run concurrently using 6 cores, leaving 2 cores free for system stability.
2.  **Phase B (Sequential):** Wall-clock timing experiments run strictly one at a time to prevent CPU cache thrashing and ensure accurate measurements.

### Usage
Make the script executable first:
`chmod +x run_all.sh`

### Script Options
*   `-h`: Show the help menu.
*   `-e <ID>`: Run a specific experiment suite (e.g., `E1`, `E2`, `E8`). Defaults to `ALL`.
*   `-a`: Automatically run the Python analysis script and generate graphs after the experiments finish.
*   `-n`: Run the experiments only, without triggering the Python analysis (Default behavior).

### Examples
Run all experiments and generate the graphs immediately:
`./run_all.sh -a`

Run only Experiment 2 (Time vs n) without triggering analysis:
`./run_all.sh -e E2 -n`

---

## Step-by-Step Guide: Running the Analysis

If you ran the experiments without the `-a` flag, or if you want to regenerate the graphs from an existing `benchmark_data.csv` file, you can run the Python script manually.

### Usage
`python3 analyze.py [OPTIONS]`

### Script Options
*   `-h`, `--help`: Show the help menu.
*   `-e <ID>`, `--experiment <ID>`: Specify which experiment to analyze and plot (e.g., `E1`, `E8`, `ALL`). Defaults to `ALL`.

### Examples
Analyze all data and output all graphs:
`python3 analyze.py`

Analyze only the crossover points (E8) and generate `e8_crossover.png`:
`python3 analyze.py -e E8`

---

## Directory Structure
*   `include/`: Header files and template implementations for the algorithms.
*   `src/`: Main entry points (`main.cpp` for the assignment CLI, `ExperimentRunner.cpp` for benchmarking).
*   `test/`: TDD test suite to verify algorithm correctness.
*   `build/`: Ignored by Git. Contains the compiled executables.