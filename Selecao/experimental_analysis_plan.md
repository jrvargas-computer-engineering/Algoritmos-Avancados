# Experimental Analysis Plan — Assignment 4 (k-th Element Selection)

This plan turns the assignment's "Experimental analysis" section into a concrete,
implementable test suite: 9 experiment groups (E1–E9), each with a fixed
objective, fixed/varied parameters, metrics, and the analysis to run on the
output. Section 0 maps every assignment requirement to the experiment(s) that
satisfy it, so nothing gets dropped from the final report.

---

## 0. Requirement → Experiment Mapping

| Assignment requirement | Experiment(s) |
|---|---|
| Comparisons & accesses as f(n) | E1 |
| Time T(n) + regression fit | E2 |
| Quickselect distribution of T, mean/variance vs n | E3 |
| Dependency on k | E4 |
| Group size g effect (theoretical + empirical) | E5 |
| Special behavior of g = 3 | E6 |
| What `std::nth_element` really does, and why | E9 |
| "Which algorithm wins in practice" / crossover points | E7, E8, E9 (synthesis) |

---

## 1. Experimental Infrastructure

### 1.1 Build configuration
Your current Makefile builds with ASan for TDD. **Add a second target** (e.g.
`make release`: `-O3 -DNDEBUG`, no `-fsanitize=address`) and use it for
*every* timing experiment (E2, E3, E5, E6, E8, E9). ASan's instrumentation
overhead (typically 2–3×, non-uniform across access patterns) would bias
absolute times and could distort which algorithm "wins" — counting
experiments (E1, E4, E7) are unaffected and may use either build.

### 1.2 Metrics extraction without breaking the I/O contract
`main.cpp`'s stdout must stay limited to the k-th element. Add a metrics
channel that doesn't touch stdout, e.g. a `--metrics` flag that prints
`comparisons=<C> accesses=<A>` to **stderr**, or writes a line to a file
given by an environment variable (`METRICS_OUT=path`). The harness reads
stdout for correctness and the side channel for metrics.

### 1.3 Timing protocol
- **Warm-up:** one untimed run per configuration before measurements start (cold cache / page-fault effects).
- **Batching for small n:** below ~1 µs per call, batch B repetitions in a loop and divide (B scaled so the batch exceeds ~50× clock resolution).
- **Repetitions:** R = 30 per (algorithm, n, k, g, distribution) cell for E2/E5/E8/E9. Report **median** (robust to OS scheduling jitter) and a bootstrap 95% CI; do *not* discard outliers in E3 — there, the tail **is** the object of study.
- **Clock:** `std::chrono::steady_clock` (monotonic).
- **Environment:** note in the report that wall-clock numbers are machine/load-dependent; mitigate with high R rather than assuming a sterile environment, unless you have control of CPU pinning / frequency scaling, in which case use it.

### 1.4 Correctness safeguard
Before logging any metric for an instance, assert that the CLI's reported result equals `NaiveSelection`'s result on the *same* instance. This catches instance-generator bugs (off-by-one k, duplicate-value tie handling) before they silently corrupt a dataset.

### 1.5 Instance generator catalog

| ID | Description | Used for |
|---|---|---|
| `UNI` | n distinct values, uniform random over `[0, 10n)` | baseline for E1–E5, E8 |
| `DUP-m` | only `m` distinct values (m = 2, 5, ⌈√n⌉), assigned randomly | duplicate stress, E7 |
| `ALLEQ` | every element identical | Lomuto degeneracy stress, E7 |
| `SORTED-ASC` / `SORTED-DESC` | already ordered | E6, E7 |
| `NEARLY-SORTED(p)` | sorted + p% random pairwise swaps (p = 1%, 5%) | E7 |
| `ORGANPIPE` | ascending then descending ("sawtooth") | classic median-of-3 trap, E7/E9 |
| `MEDIAN3-KILLER` | sequence engineered to defeat naive median-of-3 pivoting | E9 (motivates introselect) |
| `GAUSSIAN` | floating values ~ N(0,1) | realism check, E7 |

Every instance is generated **once** per (generator, n, k, seed) and reused
**identically** across all algorithms being compared in that cell (per the
assignment's "same test cases" requirement). Only the algorithm's *internal*
randomness (quickselect's pivot RNG seed) is allowed to vary independently.

### 1.6 Parameter ranges (used unless an experiment overrides them)
- **n:** 10, 20, 50, 100, 200, 500, 1e3, 2e3, 5e3, 1e4, 2e4, 5e4, 1e5, 2e5, 5e5, 1e6 (geometric-ish spacing for clean log–log fits)
- **k (relative to n):** 1 (min), 1%, 10%, 25%, 50% (median), 75%, 90%, 99%, n (max)
- **g (median-of-medians):** 3, 5, 7, 9, 11, 15, 21, 31, 51, and degenerate g = n
- **seeds:** independent stream per trial, separate from the instance-generation seed

### 1.7 CSV Logging Schema

One flat **long-format** table — one row per individual trial — rather than
per-experiment wide tables. Long format is what every analysis in Section 2
actually needs (group-by on arbitrary combinations of columns), and a single
append-only log means a harness crash mid-run loses nothing already written.
Write incrementally; filter per-experiment views downstream with `experiment_id`.

| Column | Type | Notes |
|---|---|---|
| `run_id` | int | auto-incrementing row id |
| `experiment_id` | string | `E1`…`E9` |
| `algorithm` | string | `naive`, `quickselect`, `mom`, `nth_element` |
| `n` | int | instance size |
| `k` | int | absolute rank requested |
| `k_frac` | float | `k/n`, precomputed for E4 plots |
| `g` | int, nullable | group size; empty for non-MoM rows |
| `distribution` | string | `uni`, `dup_m`, `alleq`, `sorted_asc`, `sorted_desc`, `nearly_sorted`, `organpipe`, `median3_killer`, `gaussian` |
| `dist_param` | float, nullable | e.g. `m` for `dup_m`, `p` for `nearly_sorted` |
| `instance_seed` | int | seed that generated the array (reproducibility) |
| `algo_seed` | int, nullable | quickselect's internal RNG seed; null for deterministic algorithms |
| `trial_index` | int | repetition index within the cell (0…R−1 or 0…M−1) |
| `comparisons` | int | exact count |
| `accesses` | int, nullable | null for `nth_element` (not instrumentable — §E9) |
| `recursion_depth` | int, nullable | instrumented for `mom`; optional for `quickselect` |
| `time_ns` | float | wall time, after batch-division if `batch_size` > 1 |
| `batch_size` | int | B; 1 if not batched |
| `build` | string | `release` or `asan` — keep ASan rows out of any timing analysis |
| `result_value` | numeric | the returned k-th element |
| `oracle_match` | bool | result equals `NaiveSelection`'s output on the same instance |
| `timestamp` | ISO8601 | for spotting day-to-day machine drift |
| `host` | string, optional | if runs are split across machines |

### 1.8 Analysis Toolchain

Python is the natural choice given the harness already emits CSV. Each tool
below maps to a specific need from Section 2 — the last column states the
actual derived quantity or conclusion, not just what the function computes.

| Tool / method | Applied in | Information derived |
|---|---|---|
| `pandas` groupby + agg | All | Per-cell mean/median/std/percentiles — the base summary statistic every other row in this table consumes |
| `numpy.polyfit` (log–log) | E1, E6 | Growth-rate **exponent** of comparisons vs n (≈1.0 ⇒ linear; ≈1.3–1.4 ⇒ drifting toward n log n) — this is the number that confirms or refutes an algorithm's claimed complexity class |
| `scipy.stats.linregress` | E1, E2, E6 | Slope (same exponent as above, with a CI) + R² — how confidently the data supports that exponent, used to rank candidate complexity models against each other |
| `scipy.optimize.curve_fit` (n, n·log n, n²) | E2 | Fitted constants **a, b per model** + residuals — which asymptotic class best fits real time, and the **empirical constant factor** that actually decides which algorithm is faster in practice (same complexity class ≠ same speed) |
| `scipy.optimize.curve_fit` (k-dependency formula) | E4 | Whether Quickselect's measured "hump at the median" curve matches the *magnitude* predicted by `2[n + k ln(n/k) + (n−k) ln(n/(n−k))]`, not just its visual shape |
| `scipy.stats` skew / kurtosis / `normaltest` | E3 | How non-normal/right-skewed the quickselect runtime distribution is, and whether that skew **grows or shrinks with n** (i.e. does the rare-bad-pivot tail matter more or less at scale) |
| μ(n), σ(n) via groupby + log–log fit | E3 | Whether mean and std of quickselect runtime both scale ~linearly with n (theory's prediction) → derived CV(n) = σ/μ tells you if *relative* variability is constant, growing, or shrinking |
| Recursion-depth instrumentation + groupby | E5, E6 | How max recursion depth scales with g (E5) and with n at g=3 (E6) — the **mechanistic explanation** for why g=3 underperforms, not just the symptom in the timing curve |
| `scipy.stats.bootstrap` | E8 | A **confidence interval** on the crossover n*, distinguishing a real crossover from one that's an artifact of measurement noise |
| `matplotlib` (log-scale axes, error bars, overlays) | All | Visual confirmation of every quantity above — specifically, the literal location of curve intersections for E7's degradation comparison and E8's crossover |
| `statsmodels` *(optional)* | E2 | Printed OLS summary (coefficients, p-values, CI) for the report appendix, if you want that format instead of reading `linregress` output by hand |

If you'd rather stay in R: `dplyr` + `ggplot2` cover the groupby/plotting role, and base `lm()`/`nls()` cover the linear and non-linear fits — same derived quantities, different syntax. The schema in §1.7 is language-agnostic either way.

---

## 2. Experiment Suites

### E1 — Comparisons & Accesses vs n
- **Objective:** validate growth rate of operation counts against theory (Naive ≈ Θ(n log n); Quickselect ≈ Θ(n) expected; MoM ≈ Θ(n) worst-case, larger constant).
- **Instances:** `UNI`.
- **Independent variable:** n (full range).
- **Fixed:** k = median, g = 5 (MoM baseline).
- **Metrics:** comparisons, accesses (exact counts).
- **Repetitions:** Quickselect — 50 seeds per n (average + std error). Naive/MoM are deterministic per instance; average over 10 independent `UNI` instances per n instead.
- **Analysis:** log–log plot of count vs n; linear regression on log(C) vs log(n) → estimated growth exponent with CI, compared against theoretical reference curves (n, n log n); table of accesses/comparisons ratio per algorithm (should be near-constant, e.g. ~3–4 for Lomuto due to swap cost).

### E2 — Wall-clock Time vs n + Regression Fit
- **Objective:** translate asymptotics into real constants; pick the best-fitting model per algorithm.
- **Instances:** `UNI`.
- **Independent variable:** n.
- **Fixed:** k = median, g = 5.
- **Metrics:** T(n), release build, protocol per §1.3.
- **Analysis:** fit `T = a·n + b`, `T = a·n·log(n) + b`, `T = a·n² + b` via least squares per algorithm; report R² for each candidate and select the best; report fitted (a, b) as the empirical constant factors — this is the number that actually drives the "which is faster in practice" answer, not just the asymptotic class.

### E3 — Quickselect Runtime Distribution at Fixed n
- **Objective:** characterize T (and comparisons) as a random variable: shape, mean, variance, and how each scales with n.
- **Instances:** `UNI`, fixed n ∈ {1e3, 1e4, 1e5, 1e6}.
- **Independent variable:** seed, M = 500 independent runs per fixed n.
- **Metrics:** comparisons (cheap, exact) and time.
- **Analysis:** histogram per n (overlay a normal curve for visual comparison; expect right skew from occasional unlucky pivot sequences); compute μ(n), σ²(n), σ(n), coefficient of variation σ/μ; plot μ(n) and σ(n) vs n on log–log (theory predicts both ~linear in n, i.e. CV roughly constant); report the empirical maximum over the M trials vs n as a crude tail indicator. **Do not trim outliers here** — they are the data.

### E4 — k-Dependency Study
- **Objective:** determine whether/how performance depends on the searched rank k, per algorithm.
- **Instances:** `UNI`, fixed n ∈ {1e4, 1e5, 1e6}.
- **Independent variable:** k/n sweep per §1.6 (finer granularity near the extremes and the median).
- **Fixed:** g = 5 for MoM; Quickselect averaged over 50 seeds per k.
- **Metrics:** comparisons (primary, low-noise), time (secondary check).
- **Analysis:** overlay comparisons-vs-(k/n) curves for all three algorithms. Expected pattern: Quickselect peaks near k/n = 0.5 and is cheapest at the extremes (k=1 or k=n) — fit the theoretical expected-comparisons curve `2[n + k·ln(n/k) + (n−k)·ln(n/(n−k))]` and report goodness of fit; Naive should be flat (sanity check that full-sort cost is k-independent); MoM should be comparatively flat/mildly humped given its guaranteed ~30/70 split regardless of k.

### E5 — Median-of-Medians: Group Size Sweep
- **Objective:** measure the effect of g on both the guaranteed pruning fraction (theory) and observed cost (practice); find the practically best g.
- **Instances:** `UNI`, n ∈ {1e5, 1e6}.
- **Independent variable:** g per §1.6.
- **Fixed:** k = median.
- **Metrics:** comparisons, accesses, time, **recursion depth** (instrument a counter — cheap diagnostic that explains the shape of the other curves).
- **Analysis:** plot comparisons/time vs g — expect a U-shape: small g → many recursive levels (overhead, weaker pruning guarantee — see E6); large g → expensive per-group median computation dominates (cost per group grows with g). Tabulate the theoretical elimination fraction per g (derived from "at least half the groups have median ≤ pivot") alongside the measured curve. Report the empirically optimal g* and compare it against the commonly cited g = 5 or g = 7.

### E6 — The g = 3 Anomaly (special attention, as required)
- **Objective:** explain *why* g = 3 is not guaranteed linear. The standard recurrence `T(n) ≤ T(n/g) + T(3n/4) + O(n)` only contracts when `1/g + 3/4 < 1`, i.e. **g ≥ 5**. At g = 3, `1/3 + 3/4 = 13/12 > 1` — the recursion does not provably shrink, and can degrade toward Θ(n log n) or worse.
- **Instances:** `UNI`, `SORTED-ASC`, `NEARLY-SORTED` — increasing n geometrically (10 up to 1e6), **with a hard wall-clock timeout and a recursion-depth cap** in the harness (g = 3 risks pathological depth / stack pressure — do not run unguarded at large n).
- **Independent variable:** n, with g fixed at 3.
- **Metrics:** comparisons/time vs n, recursion depth vs n.
- **Analysis:** log–log regression slope of comparisons vs n at g = 3, compared directly against the g = 5 / g = 7 slopes from E5 — quantify how far from linear it actually drifts; plot recursion depth vs n (expect faster-than-log growth relative to g ≥ 5); close with a recommendation (guard rails: minimum g, or hybrid fallback as introselect does — connects to E9).

### E7 — Robustness / Adversarial & Duplicate-Heavy Stress Test
- **Objective:** test the families designed to break specific weaknesses: `ALLEQ`/`DUP-m` (Lomuto partitions equal-to-pivot elements onto one side → potential Θ(n²) for Quickselect regardless of pivot randomness, since *every* pivot value is the same), `SORTED`/`ORGANPIPE`/`MEDIAN3-KILLER` (classic adversarial orderings), and confirm MoM's worst-case guarantee holds regardless of input order.
- **Instances:** all generators in §1.5 except `UNI`/`GAUSSIAN`, × n (moderate range — cap `ALLEQ`/`DUP` for Quickselect with a timeout, since this is exactly where it can blow up).
- **Independent variable:** instance type (categorical) × n.
- **Metrics:** comparisons, time.
- **Analysis:** grouped bar/line chart per instance type per algorithm. The key result to surface: does Quickselect degrade quadratically on `ALLEQ`/low-cardinality `DUP-m` while MoM stays flat/linear? This is the strongest empirical argument for *why a production `nth_element` can't be pure randomized quickselect* — feeds directly into E9.

### E8 — Practical Crossover Study
- **Objective:** find the n-ranges (if any) where each algorithm is fastest in wall-clock time, per the assignment's explicit ask.
- **Instances:** `UNI`, k = median (most common case).
- **Independent variable:** n, refined near any crossover bracket found from E2's fitted curves (e.g. solve `a₁·n·log n = a₂·n` for the predicted crossover n*, then sample densely around it).
- **Metrics:** time (primary), comparisons (supporting).
- **Analysis:** overlay all three T(n) curves; report crossover point(s) with a bootstrap CI; conclude with a regime table ("for n < n₁ algorithm X is fastest; …"). Treat the actual winner as an empirical question — a plausible real finding is "no crossover under uniform random input: Quickselect dominates past a small n, and MoM is preferable only under the adversarial conditions characterized in E7, not by n alone." Report whatever the data actually shows.

### E9 — `std::nth_element` Investigation (Introselect)
- **Objective:** identify and explain what the standard library actually implements, and connect it to E6/E7's findings. (Short version to validate empirically: **introselect** — quickselect with median-of-3-style pivoting, falling back to a guaranteed-linear method once recursion depth exceeds a threshold, avoiding both Quickselect's Θ(n²) worst case and MoM's constant-factor overhead in the common case.)
- **Method:**
  - *Comparisons are measurable*: `std::nth_element` accepts a custom comparator — pass a lambda that increments your `MetricsContext`, exactly as done for `NaiveSelection`'s `std::sort`.
  - *Accesses are not directly measurable* (internal swaps aren't instrumentable without modifying the standard library) — document this as a known limitation rather than reporting a fabricated number.
- **Instances:** the full E7 suite (uniform, adversarial, duplicate-heavy) plus the E2 size sweep.
- **Metrics:** time, comparisons (via instrumented comparator).
- **Analysis:** side-by-side table — does it match Quickselect's average-case speed on `UNI` while matching MoM's stability on `ALLEQ`/adversarial inputs (i.e., does it avoid the blow-up Quickselect shows in E7)? This result directly answers the assignment's "Observations" prompt.

---

## 3. Reporting Checklist

For the final report, each experiment should yield:

| Experiment | Required output |
|---|---|
| E1 | log–log count-vs-n plot + fitted exponents table |
| E2 | T(n) plot with CIs + regression table (model, a, b, R²) per algorithm |
| E3 | histograms per n + μ(n), σ(n) plots + CV discussion |
| E4 | comparisons-vs-(k/n) overlay + fit to theoretical formula |
| E5 | comparisons/time-vs-g curve + theoretical pruning-fraction table + g* |
| E6 | g=3 vs g≥5 slope comparison + recursion-depth plot + explanation of the `1/g + 3/4 < 1` condition |
| E7 | per-instance-type comparison chart, highlighting Quickselect's `ALLEQ` degradation |
| E8 | overlaid T(n) curves + crossover point(s) + regime table |
| E9 | `nth_element` vs the three algorithms across E7's instance suite + written explanation of introselect |

This set covers every bullet in the assignment's "Experimental analysis" and
"Observations" sections with a directly traceable experiment ID, while still
giving you latitude to report whatever the data actually shows (e.g. if no
true crossover exists under uniform input, E8 should say so plainly rather
than forcing one).
