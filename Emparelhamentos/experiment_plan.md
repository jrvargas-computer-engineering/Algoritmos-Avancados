# Experiment Plan — Bipartite Matching Algorithms

## Overview

The plan contains 10 tests organised across three groups, stratified along two orthogonal axes: the **algorithmic level of observation** (main loop, inner search, wall clock) and the **graph parameter being varied** (n, α, or weight range). Each test targets exactly one combination, so results are interpretable in isolation and can be composed into the unified report tables (C1 and C2).

**Repetition budget:** baseline tests use 10 repetitions, scalability tests use 20, and final report-table tests use 30. The asymmetry reflects that early tests are exploratory (checking correctness and trends) while later tests need tight confidence intervals for fitting.

**Optimisation objective:** all algorithms solve the *maximum-weight maximum matching* problem — find a matching of maximum cardinality, breaking ties in favour of maximum total edge weight. For unweighted graphs this reduces to maximum cardinality matching. The Hungarian algorithm as formulated here maximises total weight; if the underlying implementation minimises cost, edge weights must be negated before solving and the result negated back. The `matching_value` metric always records the maximum weight (positive convention).

**Production binary vs. benchmark binary:** the work specification requires that the production binary reads an undirected bipartite graph from **stdin** and writes *only* the value of the maximum (weighted) matching to **stdout**. No other output is permitted on stdout. The instrumentation described in this plan (CSV rows, observability hooks) must therefore be compiled into a separate benchmark build target (e.g. controlled by a `BENCHMARK` preprocessor flag or a CMake option). The shell orchestration script invokes the benchmark binary; the production binary is the deliverable. Both targets must link the same algorithm implementation to guarantee that benchmark results reflect the production code.

---

## Directory layout

All paths are relative to the project root and are shared with the analysis scripts and the shell orchestration script.

```
raw/
  {test_id}_{timestamp}.csv   — one file per benchmark run; written by the binary
data/
  combined/
    U1.csv, U2.csv, … C2.csv  — one file per test; produced by the shell script
                                 by concatenating and filtering the raw files
analysis_output/
  *.csv                        — derived tables produced by the analysis scripts
figures/
  *.pdf                        — publication-quality plots
```

The shell script is responsible for:
1. Running each benchmark invocation and writing output to `raw/`.
2. After all runs for a given test complete, concatenating the per-run files into
   `data/combined/{test_id}.csv` (one file per test).
3. Passing `--data-dir data/combined/` and `--out-dir analysis_output/` to the analysis entry point.

---

## CSV schema

Each benchmark invocation writes one file named `{test_id}_{timestamp}.csv`. After all runs for a test are complete, the shell script concatenates them into `data/combined/{test_id}.csv`. The universal column layout is:

```
test_id       : string  — e.g. "U1", "W3"
n             : int     — vertices per side
alpha         : float   — edge-density exponent (1.0 – 2.0)
rep           : int     — 0-based repetition index
algorithm     : string  — exact class name token (see §Algorithm name tokens)
iteration     : int     — 0-based step index within the run;
                          -1 for run-level scalar metrics
weight_regime : string  — "mixed" | "negative" | "positive" | "na"
metric_name   : string  — snake_case identifier
metric_value  : float   — raw numeric value
```

**`iteration` field:** the binary must write this column on every row. For run-level scalar metrics (e.g. `wall_clock_ms`, `matching_size`, `main_loop_iters`) it must write `-1`. For per-step time-series metrics (e.g. `defect_at_iteration`, `bfs_edges_visited` per augmentation in U2, `defect_at_iteration` per augmentation in W1) it must write the 0-based step index. Neither the analysis scripts nor the shell script may reconstruct this column from row order.

**`weight_regime` field:** the binary must write this column on every row. For all unweighted tests (U1–U4, C1) it must write `"na"`. For all weighted tests except W4 (W1–W3, C2) it must write `"mixed"`. For W4 it must write the actual regime: `"mixed"`, `"negative"`, or `"positive"` corresponding to weight ranges [−n², n²], [−n², 0], and [0, n²] respectively.

**`speedup_ratio`** is a derived quantity computed by the analysis scripts from two `wall_clock_ms` rows. The binary must **not** compute or emit it.

---

## Algorithm name tokens

| Class | `algorithm` token in CSV |
|-------|--------------------------|
| `SimpleAugmentingMatcher` | `SimpleAugmentingMatcher` |
| `HopcroftKarpMatcher` | `HopcroftKarpMatcher` |
| `HungarianMatcher + BellmanFordStrategy` | `HungarianMatcher+BellmanFordStrategy` |
| `HungarianMatcher + JohnsonDijkstraStrategy` | `HungarianMatcher+JohnsonDijkstraStrategy` |

---

## C++ observability notes

The benchmark binary must expose the following instrumentation hooks. Each hook writes one or more rows to the current output file.

- **`main_loop_iters`** (run-level, `iteration = -1`): total augmentation count for `SimpleAugmentingMatcher`; total phase count for `HopcroftKarpMatcher`.
- **`paths_found_per_phase`** (phase-level, `iteration = phase_index`): number of vertex-disjoint augmenting paths found in that phase; `HopcroftKarpMatcher` only.
- **`bfs_edges_visited`** (step-level, `iteration = augmentation_index` for Simple; `iteration = phase_index` for HK): edges visited during the BFS layer construction step.
- **`dfs_edges_visited`** (phase-level, `iteration = phase_index`): edges visited during the DFS augmentation step; `HopcroftKarpMatcher` only.
- **`defect_at_iteration`** (step-level): snapshot of the current defect (unmatched vertex count) taken at the end of each augmentation step or phase. `iteration` must match the corresponding `bfs_edges_visited` row.
- **`matching_size`** (run-level, `iteration = -1`): size of the final matching; all unweighted tests must emit this.
- **`wall_clock_ms`** (run-level, `iteration = -1`): total wall-clock time for the run, measured with a high-resolution monotonic clock.
- **`phase_count`** (run-level, `iteration = -1`): total number of BFS phases; `HopcroftKarpMatcher` only. Equivalent to `main_loop_iters` but kept as a separate metric name for clarity in U4 analysis.
- **`total_bfs_edges_visited`** (run-level, `iteration = -1`): sum of `bfs_edges_visited` across all augmentation steps/phases; used in U4.
- **`augmentation_count`** (run-level, `iteration = -1`): total number of augmenting-path calls issued by `HungarianMatcher`; written by both BF and JD strategies.
- **`matching_value`** (run-level, `iteration = -1`): total weight of the optimal matching; written by both BF and JD strategies.
- **`defect_at_iteration`** (step-level, W1 only): snapshot of outer-loop defect after each augmentation; `iteration` must equal the 0-based augmentation index.
- **`edge_relaxations`** (run-level, `iteration = -1`): total edge relaxations performed during a single pathfinding call; written per call and then summed. For analysis purposes the binary must emit the **per-call** value (one row per augmentation), so `iteration` equals the augmentation index for this metric.
- **`potential_violations_count`** (run-level, `iteration = -1`): count of transformed edge weights d'_uv < 0 detected during potential maintenance; must always be 0 for a correct `JohnsonDijkstraStrategy` implementation. Written by W2 **and** W4.
- **`potential_update_overhead_ms`** (run-level, `iteration = -1`): wall-clock time spent exclusively on Johnson potential updates (i.e., the re-weighting step after each augmentation, excluding the Dijkstra call itself). This sub-step must be timed separately using the same high-resolution monotonic clock as `wall_clock_ms`, and the value must be emitted as a run-level metric. `JohnsonDijkstraStrategy` only; written in W3 and C2.
- **`total_edge_relaxations`** (run-level, `iteration = -1`): sum of `edge_relaxations` over all augmentation calls in the run; written in C2.
- **`total_dijkstra_calls`** (run-level, `iteration = -1`): count of Dijkstra invocations in the run; written by `JohnsonDijkstraStrategy` in C2.
- **`total_edges_visited`** (run-level, `iteration = -1`): total number of edge examinations across the entire run, counting every edge inspected during any BFS layer-construction pass and any DFS augmentation pass. For `SimpleAugmentingMatcher` this is the sum of all BFS edge visits. For `HopcroftKarpMatcher` this is the sum of all BFS edge visits plus all DFS edge visits across all phases. Written in C1.

---

## Group 0 — Oracle correctness checks

### U0 · Small-instance oracle check — unweighted

**Algorithms:** `SimpleAugmentingMatcher`, `HopcroftKarpMatcher`

**Tags:** correctness

**Objective:**
Verify that both unweighted algorithms return a provably maximum-cardinality matching on small instances where exhaustive enumeration is feasible. This guards against the case where both algorithms agree on a suboptimal answer — a failure mode that the inter-algorithm consistency checks in U1–C1 cannot detect.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 2, 3, 4, 5, 6 |
| α | 1.0, 1.5, 2.0 |
| Repetitions | 50 per (n, α) pair (random graphs; some may not have a perfect matching) |

**Reference oracle:** brute-force enumeration of all subsets of edges; the maximum matching size is the maximum subset cardinality such that no vertex appears twice. Implemented in a standalone `oracle_unweighted` function, independent of the algorithm classes under test.

**Measured metrics (all run-level, `iteration = -1`):**
- `matching_size` — returned by the algorithm
- `oracle_size` — computed by the brute-force oracle for the same graph

**Pass criterion:** `matching_size == oracle_size` on every instance. Any failure halts all subsequent unweighted analysis.

**Note:** instances where the graph has no perfect matching are expected and valid; the oracle and the algorithms must agree on the actual maximum size, which may be less than n.

---

### W0 · Small-instance oracle check — weighted

**Algorithms:** `HungarianMatcher + BellmanFordStrategy`, `HungarianMatcher + JohnsonDijkstraStrategy`

**Tags:** correctness

**Objective:**
Verify that both weighted algorithms return the true maximum-weight maximum matching on small instances. Confirms the optimisation direction (maximisation) and catches negation errors.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 2, 3, 4 |
| α | 1.5 |
| Weight range | uniform integers in [−n², n²] |
| Repetitions | 100 per (n, α) pair |

**Reference oracle:** brute-force enumeration of all matchings; select the one with maximum cardinality, then among those the one with maximum total weight.

**Measured metrics (all run-level, `iteration = -1`):**
- `matching_value` — returned by the algorithm
- `oracle_value` — computed by the brute-force oracle
- `matching_size` — returned cardinality
- `oracle_size` — oracle cardinality

**Pass criterion:** `matching_size == oracle_size` and `abs(matching_value - oracle_value) < 1e-6` on every instance. Any failure halts all subsequent weighted analysis.

---

## Group 1 — Unweighted matching engines

### U1 · Main-loop iteration count vs. graph size

**Algorithms:** `SimpleAugmentingMatcher`, `HopcroftKarpMatcher`

**Tags:** main-loop level · scalability

**Objective:**
Verify that `SimpleAugmentingMatcher` executes exactly n augmentations (one per matched vertex) and that `HopcroftKarpMatcher` executes O(√n) phases, each finding multiple augmenting paths.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 50, 100, 200, 500, 1000, 2000, 5000 |
| α | 1.0, 1.5, 2.0 |
| Repetitions | 10 per (n, α) pair |

**Measured metrics (all run-level, `iteration = -1` unless noted):**
- `main_loop_iters` — augmentation count for Simple; phase count for HK
- `paths_found_per_phase` — HK only; step-level (`iteration = phase_index`)
- `matching_size` — must equal n for a perfect match (correctness check)

**Analysis:**
1. Plot `main_loop_iters` vs. n on log–log axes. Simple should show slope ≈ 1; HK slope ≈ 0.5.
2. For each α, compare curves. Edge density should not affect phase count for HK (only path length within phases).
3. Confirm `matching_size = n` on all instances as a sanity check for correctness.

---

### U2 · Augmenting-path searches vs. defect

**Algorithms:** `SimpleAugmentingMatcher`, `HopcroftKarpMatcher`

**Tags:** defect level · main-loop level

**Objective:**
Measure how the number of M-augmenting path searches evolves as the defect (number of unmatched vertices) decreases from n to 0 during a single run, isolating the inner search cost per augmentation.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 200, 500, 1000, 2000 |
| α | 1.0, 1.5, 2.0 |
| Repetitions | 5 per (n, α) pair |

**Measured metrics:**
- `defect_at_iteration` — step-level (`iteration = augmentation/phase index`); snapshot of defect at end of each step
- `bfs_edges_visited` — step-level; edges visited in the BFS layer-construction of each step
- `dfs_edges_visited` — step-level; edges visited in the DFS augmentation pass; HK only
- `matching_size` — run-level (`iteration = -1`); correctness check

**Note on `dfs_edges_visited`:** this metric is used in Output 3 of the analysis plan to decompose HK's per-phase work into layer-construction (BFS) vs. augmentation (DFS) components. It must be emitted even though it is not directly plotted on its own axis.

**Analysis:**
1. Plot `bfs_edges_visited` per search against current defect. For Simple: cost should grow as defect shrinks (paths get longer). For HK: cost per phase is bounded by m regardless of defect.
2. Overlay α = 1 and α = 2 curves to show how edge density amplifies inner-loop work for Simple but not for HK.
3. Compute the empirical ratio `bfs_cost / m` per iteration; confirm it stays bounded for HK.

---

### U3 · Wall-clock scalability — Simple vs. HopcroftKarp

**Algorithms:** `SimpleAugmentingMatcher`, `HopcroftKarpMatcher`

**Tags:** scalability

**Objective:**
Empirically confirm the O(VE) vs. O(E√V) wall-clock crossover point and quantify the practical speedup of HopcroftKarp across the full (n, α) parameter space.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 100 to 10 000, ×2 steps |
| α | 1.0, 1.25, 1.5, 1.75, 2.0 |
| Repetitions | 20 per (n, α); report median and 90th percentile |

**Measured metrics (all run-level, `iteration = -1`):**
- `wall_clock_ms` (median and p90 computed by the analysis script, not the binary)
- `matching_size` — correctness check

**Note:** `speedup_ratio` is a derived quantity (Simple_time / HK_time) computed by the analysis script from two `wall_clock_ms` rows. The binary must not emit it.

**Analysis:**
1. Log–log plot of `wall_clock_ms` vs. n for each α. Fit regression line; compare exponents to theoretical predictions.
2. Plot `speedup_ratio` as a heatmap over (n, α). Identify the minimum n at which HK is consistently faster.
3. Separate sparse (α = 1) from dense (α = 2); the advantage of HK is most visible at high density.

---

### U4 · Sensitivity to edge density — fixed n, varying α

**Algorithms:** `SimpleAugmentingMatcher`, `HopcroftKarpMatcher`

**Tags:** defect level · scalability

**Objective:**
Hold n fixed and sweep α from 1 to 2 to isolate how edge density alone affects runtime and operation counts, independent of graph size.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 500, 1000, 2000 (three fixed sizes) |
| α | 1.0, 1.1, 1.2, …, 2.0 (11 steps) |
| Repetitions | 15 per (n, α) pair |

**Measured metrics (all run-level, `iteration = -1`):**
- `wall_clock_ms`
- `total_bfs_edges_visited`
- `phase_count` (HK only)
- `matching_size` — correctness check

**Analysis:**
1. For each fixed n, plot `wall_clock_ms` vs. α. Simple should grow roughly as n^α; HK more slowly.
2. Plot `total_bfs_edges_visited` vs. α to confirm the theoretical m dependence.
3. Check whether HK `phase_count` is independent of α for a fixed n (it should be).

---

## Group 2 — Weighted matching: pathfinding comparison

### W1 · Main-loop primal defect reduction — BF vs. JD

**Algorithms:** `HungarianMatcher + BellmanFordStrategy`, `HungarianMatcher + JohnsonDijkstraStrategy`

**Tags:** main-loop level · defect level

**Objective:**
Count the number of augmenting-path calls issued by `HungarianMatcher` and verify that both pathfinding strategies produce identical defect reduction trajectories, confirming correctness before comparing cost.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 50, 100, 200, 500, 1000 |
| α | 1.0, 1.5, 2.0 |
| Weight range | uniform in [−n², n²] |
| Repetitions | 10 per (n, α) pair |

**Measured metrics:**
- `augmentation_count` — run-level (`iteration = -1`); must be identical for BF and JD
- `matching_value` — run-level (`iteration = -1`); must be identical — correctness check
- `defect_at_iteration` — step-level (`iteration = augmentation_index`); one snapshot per outer-loop step

**Analysis:**
1. Assert `augmentation_count(BF) == augmentation_count(JD)` on every instance. Any divergence indicates a bug.
2. Assert `matching_value(BF) == matching_value(JD)`. Document any numerical tolerance required.
3. Plot defect vs. iteration number; the curve shape encodes how quickly the outer loop converges.

---

### W2 · Shortest-path call cost — BF O(nm) vs. JD O(m log n)

**Algorithms:** `HungarianMatcher + BellmanFordStrategy`, `HungarianMatcher + JohnsonDijkstraStrategy`

**Tags:** main-loop level · defect level

**Objective:**
Directly measure the per-call cost of each pathfinding strategy and verify that `BellmanFordStrategy` scales as O(nm) and `JohnsonDijkstraStrategy` as O(m log n) per invocation.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 50, 100, 200, 500, 1000, 2000 |
| α | 1.0, 1.5, 2.0 |
| Weight range | uniform in [−n², n²] |
| Repetitions | 10 per (n, α) pair |

**Measured metrics:**
- `edge_relaxations` — step-level (`iteration = augmentation_index`); one row per pathfinding call so the analysis can compute per-call cost
- `potential_violations_count` — run-level (`iteration = -1`); must stay 0 for JD (all d'_uv ≥ 0)

**Analysis:**
1. Plot `edge_relaxations / call` vs. n on log–log for both strategies. BF slope should be ≈ 2 (since m = n^α, cost = nm = n^(1+α)); JD slope should be ≈ α (cost = m log n ≈ n^α log n).
2. Verify `potential_violations_count == 0` on every instance; any violation signals incorrect potential maintenance.
3. Compare slopes across α values to confirm the theoretical O(nm) vs. O(m log n) ratio.

---

### W3 · Wall-clock scalability — BF vs. JD across (n, α)

**Algorithms:** `HungarianMatcher + BellmanFordStrategy`, `HungarianMatcher + JohnsonDijkstraStrategy`

**Tags:** scalability

**Objective:**
Measure the end-to-end wall-clock time of both Hungarian variants and quantify the practical speedup of `JohnsonDijkstraStrategy`, establishing the crossover point where the potential-maintenance overhead is offset by faster per-call cost.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 50 to 2000, ×2 steps |
| α | 1.0, 1.25, 1.5, 1.75, 2.0 |
| Weight range | uniform in [−n², n²] |
| Repetitions | 20 per (n, α); report median and p90 |

**Measured metrics (all run-level, `iteration = -1`):**
- `wall_clock_ms` (median and p90 computed by the analysis script)
- `potential_update_overhead_ms` — JD only; see C++ observability note for timing requirements

**Note:** `speedup_ratio` (BF_time / JD_time) is a derived quantity computed by the analysis script. The binary must not emit it.

**Analysis:**
1. Log–log plot of `wall_clock_ms` vs. n for each α, with BF and JD as separate series.
2. Plot `speedup_ratio` vs. n for each α. Identify the minimum n where JD is consistently faster.
3. Decompose JD time into `potential_update_overhead_ms` vs. `dijkstra_ms` to confirm overhead is negligible for large n.
4. At α = 2 (dense), the advantage of JD should be most pronounced; at α = 1 (sparse), the gap narrows.

---

### W4 · Negative-weight stress test — BF correctness boundary

**Algorithms:** `HungarianMatcher + BellmanFordStrategy`, `HungarianMatcher + JohnsonDijkstraStrategy`

**Tags:** correctness · stress test

**Objective:**
Confirm that both strategies handle the full [−n², n²] weight range without failure, and measure whether the magnitude of negative weights affects convergence speed or correctness.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 100, 500, 1000 |
| α | 1.5 |
| Weight regimes | [−n², n²] (`weight_regime = "mixed"`), [−n², 0] (`weight_regime = "negative"`), [0, n²] (`weight_regime = "positive"`) |
| Repetitions | 20 per combination |

**Measured metrics (all run-level, `iteration = -1`):**
- `matching_value` — cross-check BF vs. JD
- `augmentation_count`
- `wall_clock_ms`
- `potential_violations_count` — JD only; must be 0 across all weight regimes

**Analysis:**
1. Compare `matching_value(BF)` vs. `matching_value(JD)` across all weight regimes. Flag any discrepancy.
2. Check whether all-negative weights (`weight_regime = "negative"`) produce more augmentations than mixed weights. The outer loop should be unaffected; the inner path cost is what changes.
3. Verify JD never violates d'_uv ≥ 0 even in the all-negative regime, where the initial potential p_v = −W is most stressed, using `potential_violations_count`.

---

## Group 3 — Cross-algorithm comparison

### C1 · Full unweighted algorithm head-to-head

**Algorithms:** `SimpleAugmentingMatcher`, `HopcroftKarpMatcher`

**Tags:** scalability

**Objective:**
Produce a single unified scalability table covering all algorithm–density combinations, suitable for direct inclusion in the report.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 100, 500, 1000, 2000, 5000, 10 000 |
| α | 1.0, 1.5, 2.0 |
| Repetitions | 30 per cell; report mean, std, min, max |

**Measured metrics (all run-level, `iteration = -1`):**
- `wall_clock_ms` (mean ± std, min, max)
- `main_loop_iters`
- `total_edges_visited`
- `matching_size` — correctness check

**Analysis:**
1. Produce a (n × α × algorithm) table of mean wall-clock times (`c1_scalability_table.csv`).
2. Produce an operation-count companion table from `main_loop_iters` and `total_edges_visited` (`c1_operations_table.csv`). This table provides machine-speed-independent confirmation of the complexity claims.
3. Fit power-law n^k curves for each (algorithm, α) pair; tabulate fitted exponents alongside theoretical values (`c1_exponent_table.csv`).
4. Identify any cells where Simple outperforms HK (expected only for very small n or very sparse α = 1 graphs).

---

### C2 · Full weighted algorithm head-to-head

**Algorithms:** `HungarianMatcher + BellmanFordStrategy`, `HungarianMatcher + JohnsonDijkstraStrategy`

**Tags:** scalability

**Objective:**
Produce the equivalent unified scalability table for the weighted case, enabling direct comparison between the O(n²m) and O(nm log n) total-complexity variants.

**Instances:**

| Parameter | Values |
|-----------|--------|
| n | 50, 100, 200, 500, 1000, 2000 |
| α | 1.0, 1.5, 2.0 |
| Weight range | uniform in [−n², n²] |
| Repetitions | 20 per cell; report mean, std |

**Measured metrics (all run-level, `iteration = -1`):**
- `wall_clock_ms` (mean ± std)
- `total_edge_relaxations`
- `total_dijkstra_calls` (JD only)
- `potential_update_overhead_ms` (JD only)

**Analysis:**
1. Produce a (n × α × strategy) table of mean wall-clock times for the report (`c2_scalability_table.csv`).
2. Fit power-law curves; compare empirical exponents to O(n²m) = O(n^(2+α)) for BF and O(nm log n) = O(n^(1+α) log n) for JD (`c2_exponent_table.csv`).
3. At n = 2000, α = 2, the predicted ratio BF/JD ≈ n / log(n) ≈ 182; check whether the empirical ratio is in this range using the `speedup_ratio` derived by the analysis script.

---

## Design rationale

The plan is stratified along two orthogonal axes: the algorithmic level of observation (main loop, inner search, wall clock) and the graph parameter being varied (n, α, or weight range).

Tests U1 and U2 separate the outer-loop cost from the inner search cost: U1 counts phases/augmentations as n grows, while U2 holds n fixed and traces the per-search edge cost as defect decreases within a single run. U3 converts both into wall-clock time and identifies the crossover. U4 then holds n fixed and sweeps α continuously, giving a clean view of edge-density sensitivity that is otherwise confounded in U3.

For the weighted group, W1 first establishes that both Hungarian variants produce identical outer-loop trajectories — a correctness gate before any performance claim is made. W2 measures per-call operation counts to verify the O(nm) vs. O(m log n) theory directly, and critically checks that Johnson's non-negativity invariant is never violated. W3 converts to wall-clock and finds the practical speedup, with a decomposition of potential-maintenance overhead. W4 stress-tests the negative-weight regime because that is where Bellman-Ford is doing its genuine work and where a buggy `JohnsonDijkstraStrategy` would first fail.

C1 and C2 are the report tables — wide grids of (n × α × algorithm) cells that aggregate findings from the earlier tests into a form that can be directly cited, with fitted power-law exponents compared against theoretical predictions.
