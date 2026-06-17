# Analysis Plan — Bipartite Matching Experiments

## Conventions

### Directory layout

The analysis scripts read from and write to the following paths, which are shared with the shell orchestration script. All paths are relative to the project root.

```
raw/
  {test_id}_{timestamp}.csv   — one file per benchmark run; written by the binary
data/
  combined/
    U1.csv, U2.csv, … C2.csv  — one file per test; produced by the shell script
                                 by concatenating and filtering the raw files
analysis_output/
  *.csv                        — derived tables (exponent fits, correctness reports)
figures/
  *.pdf                        — publication-quality plots
```

The analysis entry point must accept `--data-dir` (default: `data/combined/`) and `--out-dir` (default: `analysis_output/`) as command-line arguments. Figure output goes to `figures/` under `--out-dir`.

**Optimisation objective:** all `matching_value` metrics record the *maximum* total weight (positive convention). Analysis scripts must never negate `matching_value` before comparison or plotting. For unweighted tests, `matching_size` is the sole correctness criterion; for weighted tests, both `matching_size` and `matching_value` must be checked against the oracle (Outputs 0a, 0b) before any performance analysis proceeds.

**Production binary vs. benchmark binary:** the benchmark binary that writes CSV instrumentation is distinct from the production binary required by the specification (stdin graph → stdout matching value only). The analysis scripts consume benchmark binary output exclusively; they have no dependency on the production binary's interface.

**Shell script responsibilities:**
1. Each benchmark invocation writes one file to `raw/` named `{test_id}_{timestamp}.csv`.
2. After all runs for a given test complete, the script concatenates those files into `data/combined/{test_id}.csv`. The concatenation must preserve the header from the first file and strip it from subsequent files.
3. The script then invokes the analysis entry point with the appropriate `--data-dir` and `--out-dir` arguments.

### Universal CSV input schema

Every file in `data/combined/` uses the same column layout, produced by the benchmark binary and concatenated by the shell script:

```
test_id       : string  — e.g. "U1", "W3"
n             : int     — vertices per side
alpha         : float   — edge-density exponent (1.0 – 2.0)
rep           : int     — 0-based repetition index
algorithm     : string  — exact class name token (see experiment plan §Algorithm name tokens)
iteration     : int     — 0-based step index within the run;
                          -1 for run-level scalar metrics
weight_regime : string  — "mixed" | "negative" | "positive" | "na"
metric_name   : string  — snake_case identifier
metric_value  : float   — raw numeric value
```

One row per (test, instance, repetition, algorithm, iteration, metric) observation.
Multiple metrics from the same step appear as separate rows with different `metric_name` values.

**`iteration` usage in analysis scripts:** when an output requires a per-step time series (e.g. Outputs 3, 4, 10), filter to `iteration >= 0`. When an output requires run-level scalars (e.g. wall-clock time, matching size), filter to `iteration == -1`. Never assign an `iteration` column in the analysis scripts from row-index; the binary writes it directly.

**`weight_regime` usage:** W4 outputs filter on `weight_regime` directly. All other tests have `weight_regime == "na"` and the column can be ignored. The binary always writes this column; the analysis scripts never need to add it.

**`speedup_ratio`** is a derived quantity computed in the analysis scripts (simple_ms / hk_ms or BF_ms / JD_ms). It is never present in the raw CSV files.

### Aggregation conventions used throughout

- **Mean** and **std** are computed over all `rep` values for a fixed `(test_id, n, alpha, algorithm)` group.
- **Median** and **p90** are used only where the experiment plan explicitly requests them (U3, W3).
- For log–log regression, fit `log(y) = k·log(n) + c` via ordinary least squares; report `k` (the empirical exponent) and `R²`.
- For power-law labelling, theoretical exponents are derived from the complexity formulae; the empirical exponent `k` is compared directly to the theoretical one.

### Colour and marker conventions (apply consistently across all plots)

| Algorithm | Colour | Line style | Marker |
|-----------|--------|------------|--------|
| `SimpleAugmentingMatcher` | `#D85A30` (coral) | solid | circle `o` |
| `HopcroftKarpMatcher` | `#185FA5` (blue) | solid | square `s` |
| `HungarianMatcher+BellmanFordStrategy` | `#854F0B` (amber) | dashed | triangle `^` |
| `HungarianMatcher+JohnsonDijkstraStrategy` | `#0F6E56` (teal) | solid | diamond `D` |

Axes are always labelled with units in parentheses. Titles use sentence case. Grids are light (`alpha=0.3`). Legends are placed outside the plot area (upper-right or lower-right) to avoid occluding curves.

---

## Oracle correctness checks (U0, W0)

These two outputs must pass before any performance analysis is run. They are the only outputs that compare against a reference value computed independently of the algorithms under test. Both are gated: failure halts all downstream analysis.

### Output 0a · Oracle correctness table: unweighted (U0)

**Purpose:** Confirm that `SimpleAugmentingMatcher` and `HopcroftKarpMatcher` both return a provably maximum-cardinality matching on small instances, and establish the maximisation direction for the unweighted case.

**Input columns used:** `test_id="U0"`, `metric_name` in `{"matching_size", "oracle_size"}`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "U0"` and `iteration == -1`.
2. Pivot to wide: columns `returned_size` (from `matching_size`) and `oracle_size` per `(n, alpha, algorithm, rep)`.
3. Compute `correct = (returned_size == oracle_size)`.

**Table specification (written to `u0_oracle_correctness.csv`):**

Columns: `n`, `alpha`, `algorithm`, `rep`, `returned_size`, `oracle_size`, `correct`.

Summary line per algorithm: total correct / total rows. Any row with `correct == False` is printed to stderr. Any failure halts all further unweighted analysis.

**Interpretation note:** instances where `oracle_size < n` (no perfect matching exists) are valid and expected; the algorithms must match the oracle size regardless of whether it equals n.

---

### Output 0b · Oracle correctness table: weighted (W0)

**Purpose:** Confirm that both Hungarian variants return the true maximum-weight maximum matching on small instances, and verify the maximisation direction (not minimisation) for the weighted case.

**Input columns used:** `test_id="W0"`, `metric_name` in `{"matching_size", "oracle_size", "matching_value", "oracle_value"}`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "W0"` and `iteration == -1`.
2. Pivot to wide: columns `ret_size`, `ora_size`, `ret_value`, `ora_value` per `(n, alpha, algorithm, rep)`.
3. Compute `size_correct = (ret_size == ora_size)` and `value_correct = abs(ret_value - ora_value) < 1e-6`.

**Table specification (written to `w0_oracle_correctness.csv`):**

Columns: `n`, `alpha`, `algorithm`, `rep`, `ret_size`, `ora_size`, `size_correct`, `ret_value`, `ora_value`, `value_correct`.

Summary line per algorithm: total fully-correct rows / total rows. Any row failing either check is printed to stderr. Any failure halts all further weighted analysis.

---

## Test U1 — Main-loop iteration count vs. graph size

### Output 1 · Log–log line plot: `main_loop_iters` vs. n

**Purpose:** Confirm O(n) augmentation count for Simple and O(√n) phase count for HK.

**Input columns used:** `test_id="U1"`, `metric_name="main_loop_iters"`, `n`, `alpha`, `algorithm`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter rows to `test_id == "U1"`, `metric_name == "main_loop_iters"`, and `iteration == -1`.
2. Group by `(n, alpha, algorithm)`; compute `mean` and `std` over `rep`.

**Plot specification:**
- One subplot per `alpha` value (three subplots: 1.0, 1.5, 2.0), arranged in a single row.
- X-axis: `n`, log scale. Label: `"n (vertices per side)"`.
- Y-axis: `mean(main_loop_iters)`, log scale. Label: `"Mean main-loop iterations"`.
- Error bars: ± 1 std (use `errorbar` or equivalent).
- For each algorithm, overlay a reference line `y = A · n^k_theory` where `k_theory = 1.0` for Simple and `k_theory = 0.5` for HK; fit `A` by minimising squared error.
- Annotation on each curve: fitted empirical exponent `k_emp` rounded to 2 decimal places, placed at the right end of the curve.
- Colour and marker per algorithm as per conventions.

**Table companion — fitted exponents:**

| alpha | Algorithm | k_theory | k_emp | R² |
|-------|-----------|----------|-------|----|
| 1.0 | SimpleAugmentingMatcher | 1.0 | … | … |
| 1.0 | HopcroftKarpMatcher | 0.5 | … | … |
| … | … | … | … | … |

Table is written to `u1_fitted_exponents.csv`.

**Interpretation notes:**
- If `k_emp` for Simple deviates more than 0.1 from 1.0, re-check the augmentation counter for off-by-one errors.
- HK `main_loop_iters` (phase count) should be the same across all three α subplots for a fixed n — any α-dependence indicates a layered-network construction bug.

---

### Output 2 · Correctness table: `matching_size` == n

**Purpose:** Sanity check that both algorithms produce perfect matchings on all instances.

**Input columns used:** `test_id="U1"`, `metric_name="matching_size"`, `n`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "U1"`, `metric_name == "matching_size"`, and `iteration == -1`.
2. For each row, compute `correct = (metric_value == n)`.
3. Group by `(algorithm, n, alpha)`; report `sum(correct)` and `total = count(rep)`.

**Table specification:**

Columns: `algorithm`, `n`, `alpha`, `correct_runs`, `total_runs`, `pass` (boolean: `correct_runs == total_runs`).

Rows failing `pass` must be printed to stderr and halt subsequent analysis for that algorithm.

**Note:** this check confirms that the algorithms find a *perfect* matching when one exists (i.e. when `oracle_size == n`). On instances where no perfect matching exists, `matching_size < n` is correct and `pass` will be false; those rows should be cross-referenced against `u0_oracle_correctness.csv` rather than treated as failures. The correctness module in `correctness.py` should accept a `test_id` parameter so it can be reused across all unweighted tests. The definitive optimality gate is Output 0a; this output monitors consistency at larger n where the oracle is not run.

---

## Test U2 — Augmenting-path searches vs. defect

### Output 3 · Line plot: `bfs_edges_visited` vs. defect (within-run trace)

**Purpose:** Show how inner-loop work evolves as the matching grows, distinguishing Simple (cost grows) from HK (cost bounded).

**Input columns used:** `test_id="U2"`, `metric_name` in `{"bfs_edges_visited", "dfs_edges_visited", "defect_at_iteration"}`, `n`, `alpha`, `algorithm`, `rep`, `iteration`, `metric_value`

**Preparation:**
1. Filter to `test_id == "U2"`, `metric_name` in `{"bfs_edges_visited", "dfs_edges_visited", "defect_at_iteration"}`, and `iteration >= 0`.
2. The `iteration` column is already present (written by the binary). Do not re-derive it from row index.
3. Pivot so each `(n, alpha, algorithm, rep, iteration)` group has `bfs_edges_visited`, `dfs_edges_visited`, and `defect_at_iteration` as separate columns. Rows missing `dfs_edges_visited` (Simple rows) receive `NaN`.
4. Compute `mean_bfs`, `mean_dfs`, and `mean_defect` over `rep` for each `(n, alpha, algorithm, iteration)`.

**Plot specification:**
- One figure per `(n, alpha)` pair (4 × 3 = 12 subplots, arranged in a 4×3 grid).
- X-axis: `mean_defect` (decreasing left-to-right since defect shrinks). Label: `"Defect (unmatched vertices)"`.
- Y-axis: `mean_bfs_edges_visited`. Label: `"Mean BFS edges visited"`.
- One line per algorithm per subplot; colour and marker per conventions.
- For HK subplots, overlay a second line for `mean_dfs_edges_visited` using the same colour but a dotted line style and a `×` marker, labelled `"HK DFS"`. This decomposes the total phase work into layer-construction (BFS) and augmentation (DFS) components.
- X-axis is reversed (`xlim` from max defect to 0) so time flows left to right.

**Interpretation notes:**
- For Simple, the curve should rise as defect approaches 0 (last augmentations traverse the longest paths).
- For HK, the curve should remain roughly flat across all defect values; a rising tail indicates the DFS phase is not reusing the BFS layers correctly.

---

### Output 4 · Line plot: empirical ratio `bfs_edges_visited / m` per iteration

**Purpose:** Confirm HK's per-phase cost stays O(m), i.e. the ratio remains bounded while Simple's grows.

**Input columns used:** same as Output 3, plus `alpha` to derive `m = n^alpha`.

**Preparation:**
1. Use the same pivoted dataset from Output 3.
2. Compute `m_approx = round(n ** alpha)`.
3. Compute `ratio = mean_bfs / m_approx` per `(n, alpha, algorithm, iteration)`.

**Plot specification:**
- Same facet layout as Output 3 (one subplot per `(n, alpha)`).
- X-axis: `mean_defect` (reversed).
- Y-axis: `ratio = bfs_edges_visited / m`. Label: `"bfs_edges_visited / m"`.
- Add a horizontal reference line at `y = 1.0` labelled `"m (theory bound)"`.
- HK curve should stay below or at the reference line; Simple's curve should rise above it for large n.

---

## Test U3 — Wall-clock scalability: Simple vs. HopcroftKarp

### Output 5 · Log–log line plot: `wall_clock_ms` vs. n, faceted by α

**Purpose:** Show the empirical scalability curves for both algorithms and confirm the crossover point.

**Input columns used:** `test_id="U3"`, `metric_name="wall_clock_ms"`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "U3"`, `metric_name == "wall_clock_ms"`, and `iteration == -1`.
2. Group by `(n, alpha, algorithm)`; compute `median` and `p90` over `rep`.

**Plot specification:**
- One subplot per `alpha` (five subplots: 1.0, 1.25, 1.5, 1.75, 2.0), arranged in a 2×3 grid (last cell empty).
- X-axis: `n`, log scale. Label: `"n (vertices per side)"`.
- Y-axis: `median(wall_clock_ms)`, log scale. Label: `"Wall-clock time (ms), median"`.
- Shaded band between median and p90 for each algorithm (same colour, 20% opacity).
- Overlay theoretical reference lines: Simple ∝ n^(1+α), HK ∝ n^(0.5+α); fit the constant by least squares.
- Annotate the crossover n value on each subplot (the smallest n where `median_HK < median_Simple`).

**Table companion — fitted exponents:**

| alpha | Algorithm | k_theory | k_emp | R² | Crossover n |
|-------|-----------|----------|-------|----|-------------|
| 1.00 | SimpleAugmentingMatcher | 2.00 | … | … | … |
| 1.00 | HopcroftKarpMatcher | 1.50 | … | … | … |
| … | … | … | … | … | … |

Written to `u3_fitted_exponents.csv`.

---

### Output 6 · Heatmap: `speedup_ratio` over (n, α)

**Purpose:** Give a two-dimensional view of where HK's advantage is largest.

**Input columns used:** `test_id="U3"`, `metric_name="wall_clock_ms"`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Compute `median_time` per `(n, alpha, algorithm)`.
2. Pivot to wide format: columns `simple_ms` and `hk_ms`.
3. Compute `speedup_ratio = simple_ms / hk_ms` (derived quantity; not present in the raw files).

**Plot specification:**
- Single heatmap. Rows: `alpha` values (low at bottom). Columns: `n` values (increasing left to right).
- Colour: `speedup_ratio`; use a sequential colormap (e.g. `YlOrRd`). Centre the colorbar at `1.0` (no speedup); values below 1.0 indicate Simple is faster.
- Annotate each cell with the ratio rounded to 1 decimal place.
- Colourbar label: `"Speedup (Simple / HK)"`.
- Draw a bold cell border on every cell where `speedup_ratio >= 1.0` to visually mark the HK-favourable region.

---

## Test U4 — Sensitivity to edge density (fixed n, varying α)

### Output 7 · Line plot: `wall_clock_ms` vs. α, one line per algorithm, faceted by n

**Purpose:** Isolate the effect of edge density on runtime for a fixed graph size.

**Input columns used:** `test_id="U4"`, `metric_name="wall_clock_ms"`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "U4"`, `metric_name == "wall_clock_ms"`, and `iteration == -1`.
2. Group by `(n, alpha, algorithm)`; compute `mean` and `std`.

**Plot specification:**
- One subplot per fixed `n` value (three subplots): 500, 1000, 2000.
- X-axis: `alpha`, linear scale, range [1.0, 2.0]. Label: `"α (edge-density exponent)"`.
- Y-axis: `mean(wall_clock_ms)`, linear scale. Label: `"Mean wall-clock time (ms)"`.
- Error band: mean ± 1 std (shaded, 20% opacity).
- Each subplot title: `"n = {n}"`.

---

### Output 8 · Line plot: `phase_count` vs. α for HK only

**Purpose:** Confirm that HK phase count is independent of α (it depends only on n).

**Input columns used:** `test_id="U4"`, `metric_name="phase_count"`, `n`, `alpha`, `algorithm="HopcroftKarpMatcher"`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "U4"`, `metric_name == "phase_count"`, `algorithm == "HopcroftKarpMatcher"`, and `iteration == -1`.
2. Group by `(n, alpha)`; compute `mean` and `std`.

**Plot specification:**
- Single plot; one line per `n` value.
- X-axis: `alpha`, linear, [1.0, 2.0]. Label: `"α"`.
- Y-axis: `mean(phase_count)`. Label: `"Mean phase count (HK)"`.
- Lines should appear horizontal (flat), confirming α-independence. Any slope > 0.05 in a linear fit of `phase_count ~ alpha` should be flagged in a printed warning.

---

## Test W1 — Main-loop defect reduction: BF vs. JD

### Output 9 · Correctness assertion table: `augmentation_count` and `matching_value`

**Purpose:** Gate for all subsequent weighted-graph analysis — both strategies must agree exactly. Also confirms that both strategies maximise weight (not minimise): if their values agree but disagree with the oracle from Output 0b, a systematic negation error is present.

**Input columns used:** `test_id="W1"`, `metric_name` in `{"augmentation_count", "matching_value"}`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `iteration == -1`.
2. Pivot to wide format: one row per `(n, alpha, rep)`, columns `aug_BF`, `aug_JD`, `val_BF`, `val_JD`.
3. Compute `aug_match = (aug_BF == aug_JD)` and `val_match = abs(val_BF - val_JD) < 1e-6`.

**Table specification:**

Columns: `n`, `alpha`, `rep`, `aug_BF`, `aug_JD`, `aug_match`, `val_BF`, `val_JD`, `val_match`.

Rows where `aug_match == False` or `val_match == False` are printed to stderr as failures. The table is written to `w1_correctness.csv`. A summary row is added at the bottom: `"PASS"` if all rows pass, `"FAIL (k failures)"` otherwise.

---

### Output 10 · Line plot: defect vs. iteration for selected (n, α)

**Purpose:** Show the outer-loop convergence profile; the two strategies must produce identical curves (overlapping lines confirm correctness visually).

**Input columns used:** `test_id="W1"`, `metric_name="defect_at_iteration"`, `n`, `alpha`, `algorithm`, `rep`, `iteration`, `metric_value`

**Preparation:**
1. Filter to `test_id == "W1"`, `metric_name == "defect_at_iteration"`, and `iteration >= 0`.
2. The `iteration` column is already present in the file. Do not re-derive it from row index.
3. Group by `(n, alpha, algorithm, iteration)`; compute `mean_defect` over `rep`.
4. Select representative subset: `n` ∈ {100, 500, 1000}, `alpha` ∈ {1.0, 2.0} → 6 subplots.

**Plot specification:**
- 2×3 subplot grid (rows: α = 1.0, 2.0; columns: n = 100, 500, 1000).
- X-axis: `iteration`. Label: `"Augmentation step"`.
- Y-axis: `mean_defect`. Label: `"Defect (unmatched vertices)"`.
- Both algorithm curves plotted on each subplot; they must visually overlap. If they diverge, print a warning identifying the `(n, alpha)` pair.

---

## Test W2 — Shortest-path call cost: BF O(nm) vs. JD O(m log n)

### Output 11 · Log–log line plot: `edge_relaxations` per call vs. n, faceted by α

**Purpose:** Empirically verify the per-call complexity of each pathfinding strategy.

**Input columns used:** `test_id="W2"`, `metric_name="edge_relaxations"`, `n`, `alpha`, `algorithm`, `rep`, `iteration`, `metric_value`

**Preparation:**
1. Filter to `test_id == "W2"`, `metric_name == "edge_relaxations"`, and `iteration >= 0`.
2. Average `metric_value` over all iterations (augmentation calls) within each `(n, alpha, algorithm, rep)` to obtain a per-call mean.
3. Group by `(n, alpha, algorithm)`; compute `mean` and `std` of the per-call mean over `rep`.

**Plot specification:**
- One subplot per `alpha` (three subplots: 1.0, 1.5, 2.0).
- X-axis: `n`, log scale. Label: `"n"`.
- Y-axis: `mean(edge_relaxations per call)`, log scale. Label: `"Mean edge relaxations per call"`.
- Overlay theoretical lines:
  - BF: `y = A · n^(1 + alpha)` (since `m = n^alpha`, cost = `n · m`).
  - JD: `y = B · n^alpha · log2(n)` (plotted as a curve, not a straight log–log line).
- Annotate fitted empirical exponents on each curve.

**Table companion — fitted exponents:**

| alpha | Algorithm | k_theory | k_emp | R² |
|-------|-----------|----------|-------|----|
| 1.0 | BellmanFordStrategy | 2.0 | … | … |
| 1.0 | JohnsonDijkstraStrategy | ~1.0 | … | … |
| … | … | … | … | … |

Note: JD's theoretical curve is not a pure power law (it includes a log n factor), so `k_theory` for JD is listed as `alpha` (the dominant exponent, ignoring the log factor). The fit residual will be slightly worse than for BF.

Written to `w2_fitted_exponents.csv`.

---

### Output 12 · Correctness table: `potential_violations_count`

**Purpose:** Confirm that JohnsonDijkstraStrategy never produces a negative transformed edge weight.

**Input columns used:** `test_id="W2"`, `metric_name="potential_violations_count"`, `algorithm="HungarianMatcher+JohnsonDijkstraStrategy"`, `n`, `alpha`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "W2"`, `metric_name == "potential_violations_count"`, and `iteration == -1`.
2. Any row with `metric_value > 0` is a failure.

**Table specification:**

Columns: `n`, `alpha`, `rep`, `potential_violations_count`, `pass`.

A summary line is printed: total failures / total rows. Written to `w2_potential_violations.csv`. Any failure halts further JD analysis with an error message.

---

## Test W3 — Wall-clock scalability: BF vs. JD

### Output 13 · Log–log line plot: `wall_clock_ms` vs. n, faceted by α

**Purpose:** Primary scalability comparison for the weighted case; equivalent to Output 5 for unweighted.

**Input columns used:** `test_id="W3"`, `metric_name="wall_clock_ms"`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "W3"`, `metric_name == "wall_clock_ms"`, and `iteration == -1`.
2. Group by `(n, alpha, algorithm)`; compute `median` and `p90`.

**Plot specification:**
- One subplot per `alpha` (five subplots: 1.0, 1.25, 1.5, 1.75, 2.0) in a 2×3 grid.
- X-axis: `n`, log scale. Label: `"n"`.
- Y-axis: `median(wall_clock_ms)`, log scale. Label: `"Wall-clock time (ms), median"`.
- Shaded p90 band as in Output 5.
- Theoretical reference lines:
  - BF total: ∝ n^(2+α) (n augmentations × nm per BF call).
  - JD total: ∝ n^(1+α) · log n.
- Annotate crossover n on each subplot.

**Table companion:**

Same structure as the U3 table, written to `w3_fitted_exponents.csv`.

---

### Output 14 · Heatmap: `speedup_ratio` (BF / JD) over (n, α)

**Purpose:** Two-dimensional view of JD speedup in the weighted case; directly comparable to Output 6.

**Preparation and plot specification:** identical to Output 6, substituting `test_id="W3"` and the weighted algorithm names. `speedup_ratio` is computed as `BF_median_ms / JD_median_ms` in the analysis script. Written to the same layout with colourbar label `"Speedup (BF / JD)"`.

---

### Output 15 · Stacked bar chart: JD time decomposition

**Purpose:** Show that `potential_update_overhead_ms` is a negligible fraction of total JD time for large n.

**Input columns used:** `test_id="W3"`, `metric_name` in `{"wall_clock_ms", "potential_update_overhead_ms"}`, `algorithm` containing `"JohnsonDijkstra"`, `n`, `alpha`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "W3"`, JD algorithm only, and `iteration == -1`.
2. Pivot: columns `total_ms` (from `wall_clock_ms`) and `overhead_ms` (from `potential_update_overhead_ms`).
3. Compute `dijkstra_ms = total_ms - overhead_ms`.
4. Group by `(n, alpha)`; compute means.
5. Select one representative α value (α = 1.5) for the main figure; include the others in supplementary output.

**Plot specification:**
- X-axis: `n` values (categorical, ordered). Label: `"n"`.
- Y-axis: time in ms. Label: `"Time (ms)"`.
- Two stacked bars per n: `dijkstra_ms` (teal, bottom) and `overhead_ms` (amber, top).
- Annotate each bar with the overhead fraction as a percentage (e.g. `"3.2%"`).
- Expected result: overhead fraction decreases with n; confirm this trend with a printed note.

---

## Test W4 — Negative-weight stress test

### Output 16 · Grouped bar chart: `wall_clock_ms` by weight regime

**Purpose:** Compare runtime and correctness across the three weight regimes ([−n², n²], [−n², 0], [0, n²]).

**Input columns used:** `test_id="W4"`, `metric_name="wall_clock_ms"`, `n`, `alpha`, `algorithm`, `weight_regime`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "W4"`, `metric_name == "wall_clock_ms"`, and `iteration == -1`.
2. Group by `(n, algorithm, weight_regime)`; compute `mean` and `std`.

**Plot specification:**
- One subplot per `n` value (three subplots: 100, 500, 1000).
- X-axis: `weight_regime` (three categories: `"mixed"`, `"negative"`, `"positive"`). Label: `"Weight regime"`.
- Y-axis: `mean(wall_clock_ms)`. Label: `"Mean wall-clock time (ms)"`.
- Two bars per category: one per algorithm (BF = amber, JD = teal). Error bars: ± 1 std.

---

### Output 17 · Correctness difference table: `matching_value` across weight regimes

**Purpose:** Confirm BF and JD produce identical optimal values across all weight regimes.

**Input columns used:** `test_id="W4"`, `metric_name="matching_value"`, `n`, `algorithm`, `weight_regime`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `iteration == -1`.
2. Pivot to wide: columns `val_BF` and `val_JD` per `(n, weight_regime, rep)`.
3. Compute `abs_diff = abs(val_BF - val_JD)` and `pass = abs_diff < 1e-6`.

**Table specification:**

Columns: `n`, `weight_regime`, `rep`, `val_BF`, `val_JD`, `abs_diff`, `pass`.

Summary line: total failures per weight regime. Written to `w4_correctness.csv`.

---

### Output 17b · Correctness table: `potential_violations_count` for W4

**Purpose:** Confirm that `JohnsonDijkstraStrategy` never violates d'_uv ≥ 0 across all three weight regimes, with particular attention to the all-negative regime where the initial potential p_v = −W is most stressed.

**Input columns used:** `test_id="W4"`, `metric_name="potential_violations_count"`, `algorithm="HungarianMatcher+JohnsonDijkstraStrategy"`, `n`, `weight_regime`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "W4"`, `metric_name == "potential_violations_count"`, and `iteration == -1`.
2. Any row with `metric_value > 0` is a failure.

**Table specification:**

Columns: `n`, `weight_regime`, `rep`, `potential_violations_count`, `pass`.

Group failures by `weight_regime` in the summary line. Written to `w4_potential_violations.csv`. Any failure halts further JD analysis for the affected weight regime with an error message. This check must run before Output 16.

---

## Tests C1 and C2 — Unified head-to-head tables

### Output 18 · Scalability summary table: unweighted (C1)

**Purpose:** Final report table for unweighted algorithms; a single artefact covering all (n, α, algorithm) cells.

**Input columns used:** `test_id="C1"`, `metric_name="wall_clock_ms"`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `test_id == "C1"`, `metric_name == "wall_clock_ms"`, and `iteration == -1`.
2. Group by `(n, alpha, algorithm)`; compute `mean`, `std`, `min`, `max`.
3. Pivot to wide: index = `(n, alpha)`, columns = `(algorithm, statistic)`.

**Table specification (written to `c1_scalability_table.csv`):**

| n | alpha | Simple_mean_ms | Simple_std_ms | HK_mean_ms | HK_std_ms | speedup_ratio |
|---|-------|----------------|---------------|------------|-----------|---------------|
| 100 | 1.0 | … | … | … | … | … |
| … | … | … | … | … | … | … |

Where `speedup_ratio = Simple_mean_ms / HK_mean_ms` (computed in the analysis script).

**Operation-count companion table (written to `c1_operations_table.csv`):**

Computed from `metric_name` in `{"main_loop_iters", "total_edges_visited"}` from the same C1 data, filtered to `iteration == -1`.

| n | alpha | Simple_mean_iters | HK_mean_iters | Simple_mean_edges | HK_mean_edges |
|---|-------|------------------|---------------|-------------------|---------------|
| … | … | … | … | … | … |

This table provides machine-speed-independent confirmation of the complexity claims alongside the wall-clock table.

**Companion figure:** reproduce Output 5 with C1 data (larger repetition count gives tighter bands) at publication quality (300 dpi, 6×8 inches). Save as `c1_scalability_plot.pdf`.

---

### Output 19 · Fitted exponent comparison table: unweighted (C1)

**Purpose:** Quantitative evidence that empirical exponents match theoretical predictions.

**Preparation:**
1. For each `(alpha, algorithm)` combination, fit `log(mean_ms) = k · log(n) + c` using all n values.
2. Record `k_emp`, `c`, and `R²`.
3. Look up `k_theory`:
   - Simple: `k_theory = 1.0 + alpha`
   - HK: `k_theory = 0.5 + alpha`

**Table specification (written to `c1_exponent_table.csv`):**

| alpha | Algorithm | k_theory | k_emp | delta_k | R² |
|-------|-----------|----------|-------|---------|-----|
| 1.00 | SimpleAugmentingMatcher | 2.00 | … | … | … |
| 1.00 | HopcroftKarpMatcher | 1.50 | … | … | … |
| … | … | … | … | … | … |

Where `delta_k = k_emp - k_theory`. Flag rows where `abs(delta_k) > 0.15`.

---

### Output 20 · Scalability summary table: weighted (C2)

**Purpose:** Final report table for the Hungarian variants; equivalent to Output 18 for the weighted case.

**Input columns used:** `test_id="C2"`, `metric_name="wall_clock_ms"`, `n`, `alpha`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:** identical to Output 18, substituting the two Hungarian algorithm names.

**Table specification (written to `c2_scalability_table.csv`):**

| n | alpha | BF_mean_ms | BF_std_ms | JD_mean_ms | JD_std_ms | speedup_ratio |
|---|-------|------------|-----------|------------|-----------|---------------|
| … | … | … | … | … | … | … |

**Operation-count companion table (written to `c2_operations_table.csv`):**

Computed from `metric_name` in `{"total_edge_relaxations", "total_dijkstra_calls"}`, filtered to `iteration == -1`. `total_dijkstra_calls` rows appear only for `algorithm == "HungarianMatcher+JohnsonDijkstraStrategy"`; fill with `NaN` for BF rows.

| n | alpha | BF_mean_relaxations | JD_mean_relaxations | JD_mean_dijkstra_calls |
|---|-------|---------------------|---------------------|------------------------|
| … | … | … | … | … |

This table provides machine-speed-independent confirmation of the O(n²m) vs. O(nm log n) complexity claims alongside the wall-clock table.

**Companion figure:** reproduce Output 13 with C2 data at publication quality. Save as `c2_scalability_plot.pdf`.

---

### Output 21 · Fitted exponent comparison table: weighted (C2)

**Purpose:** Quantitative evidence for the O(n²m) vs. O(nm log n) complexity claims.

**Preparation:**
1. Fit `log(mean_ms) = k · log(n) + c` per `(alpha, algorithm)`.
2. Theoretical exponents:
   - BF: `k_theory = 2.0 + alpha` (n augmentations × O(nm) per call = O(n²m))
   - JD: `k_theory = 1.0 + alpha` (dominant term of O(nm log n), log factor excluded from fit)

**Table specification (written to `c2_exponent_table.csv`):**

| alpha | Algorithm | k_theory | k_emp | delta_k | R² | note |
|-------|-----------|----------|-------|---------|-----|------|
| 1.00 | BellmanFordStrategy | 3.00 | … | … | … | |
| 1.00 | JohnsonDijkstraStrategy | 2.00 | … | … | … | log n excluded from fit |
| … | … | … | … | … | … | … |

Flag rows where `abs(delta_k) > 0.20` (looser tolerance for JD since the log factor causes systematic underestimation of the empirical exponent).

---

### Output 22 · Large-n ratio validation: BF / JD speedup vs. predicted n / log(n)

**Purpose:** At large n and α = 2, the theoretical speedup ratio is approximately n / log₂(n). This output checks whether the empirical speedup approaches this prediction.

**Input columns used:** `test_id="C2"`, `metric_name="wall_clock_ms"`, `n`, `alpha=2.0`, `algorithm`, `rep`, `iteration==-1`, `metric_value`

**Preparation:**
1. Filter to `alpha == 2.0` and `iteration == -1`.
2. Compute `empirical_speedup = BF_mean_ms / JD_mean_ms` per n (derived in analysis script).
3. Compute `predicted_speedup = n / log2(n)` per n.

**Plot specification:**
- Single plot.
- X-axis: `n`, log scale. Label: `"n"`.
- Y-axis: speedup ratio, linear scale. Label: `"Speedup ratio"`.
- Two series: `empirical_speedup` (teal diamonds) and `predicted_speedup` (gray dashed line labelled `"n / log₂(n)"`).
- Annotate the ratio `empirical / predicted` at the largest n value as a percentage of the theoretical bound.

---

## Output file inventory

| File | Produced by | Description |
|------|-------------|-------------|
| `u0_oracle_correctness.csv` | Output 0a | Unweighted oracle vs. algorithm correctness |
| `w0_oracle_correctness.csv` | Output 0b | Weighted oracle vs. algorithm correctness |
| `u1_fitted_exponents.csv` | Output 1 | Exponents for Simple and HK iteration counts |
| `u3_fitted_exponents.csv` | Output 5 | Exponents for unweighted wall-clock times |
| `w2_fitted_exponents.csv` | Output 11 | Exponents for per-call edge relaxations |
| `w2_potential_violations.csv` | Output 12 | JD correctness check (W2) |
| `w3_fitted_exponents.csv` | Output 13 | Exponents for weighted wall-clock times |
| `w4_correctness.csv` | Output 17 | BF vs. JD value agreement per weight regime |
| `w4_potential_violations.csv` | Output 17b | JD potential-violation check per weight regime |
| `w1_correctness.csv` | Output 9 | BF vs. JD augmentation and value agreement |
| `c1_scalability_table.csv` | Output 18 | Full unweighted timing table |
| `c1_operations_table.csv` | Output 18 | Unweighted operation-count companion table |
| `c1_exponent_table.csv` | Output 19 | Fitted exponents for unweighted case |
| `c2_scalability_table.csv` | Output 20 | Full weighted timing table |
| `c2_operations_table.csv` | Output 20 | Weighted operation-count companion table |
| `c2_exponent_table.csv` | Output 21 | Fitted exponents for weighted case |
| `c1_scalability_plot.pdf` | Output 18 | Publication-quality unweighted scalability figure |
| `c2_scalability_plot.pdf` | Output 20 | Publication-quality weighted scalability figure |

All intermediate `.csv` outputs should be placed in an `analysis_output/` directory. All `.pdf` figures in `figures/`.

---

## Recommended script structure

A developer implementing the analysis script should organise it as follows:

```
analysis/
  load.py          — reads all raw CSVs from data/combined/, returns a unified DataFrame
  aggregate.py     — groupby helpers, median/p90/mean/std computation, pivot utilities
  fit.py           — log-log regression, power-law fitting, R² computation
  correctness.py   — assertion helpers used by Outputs 0a, 0b, 2, 9, 12, 17, 17b;
                     accepts a test_id parameter for reuse across unweighted tests
  oracle.py        — brute-force reference implementations for unweighted (U0) and
                     weighted (W0) small instances; independent of algorithm classes
  plots/
    u0.py          — Output 0a (oracle unweighted)
    w0.py          — Output 0b (oracle weighted)
    u1.py          — Outputs 1, 2
    u2.py          — Outputs 3, 4
    u3.py          — Outputs 5, 6
    u4.py          — Outputs 7, 8
    w1.py          — Outputs 9, 10
    w2.py          — Outputs 11, 12
    w3.py          — Outputs 13, 14, 15
    w4.py          — Outputs 16, 17, 17b
    c1.py          — Outputs 18, 19
    c2.py          — Outputs 20, 21, 22
  run_all.py       — imports all plot modules and runs them in dependency order:
                     1. Output 0a (oracle unweighted) — abort all unweighted if failed
                     2. Output 0b (oracle weighted)   — abort all weighted if failed
                     3. Output 2  (matching_size consistency, U1)
                     4. Output 9  (augmentation/value parity, W1)
                     5. Output 12 (potential violations, W2)
                     6. Output 17b (potential violations, W4)
                     7. All remaining outputs in test order
```

The script should accept a `--data-dir` argument pointing to the raw CSV directory and a `--out-dir` argument for outputs. All figure parameters (DPI, figure size, font size) should be centralised in a single `config.py` file rather than hardcoded per plot.
