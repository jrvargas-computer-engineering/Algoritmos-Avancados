#!/usr/bin/env python3
"""Analysis for the k-th element selection experiments (E1-E9).

Reads the long-format log written by build/run_experiments
(data/benchmark_data.csv, schema in experimental_analysis_plan.md §1.7) and
produces, per experiment, the tables (CSV) and figures (PNG) listed in the
plan's Reporting Checklist (§3). Outputs go to analysis_output/.

Every experiment's analysis is run inside a guard: missing data (e.g. a cell
that hit the run_all.sh timeout) prints a note and is skipped rather than
aborting the rest of the analysis.
"""
import argparse
import os

import numpy as np
import pandas as pd

import matplotlib
matplotlib.use("Agg")  # headless: write PNGs without a display
import matplotlib.pyplot as plt

from scipy.optimize import curve_fit
from scipy.stats import linregress, skew, kurtosis

CSV_PATH = "data/benchmark_data.csv"
OUT_DIR = "analysis_output"
FIG_DIR = os.path.join(OUT_DIR, "figures")

ALGO_ORDER = ["naive", "quickselect", "mom", "nth_element"]


# --------------------------------------------------------------------------
# helpers
# --------------------------------------------------------------------------
def ensure_dirs():
    os.makedirs(FIG_DIR, exist_ok=True)


def save_fig(fig, name):
    path = os.path.join(FIG_DIR, name)
    fig.savefig(path, dpi=120, bbox_inches="tight")
    plt.close(fig)
    print(f"  figure -> {path}")


def save_table(df, name):
    path = os.path.join(OUT_DIR, name)
    df.to_csv(path, index=False)
    print(f"  table  -> {path}")


def subset(df, exp):
    return df[df["experiment_id"] == exp].copy()


def loglog_exponent(x, y):
    """Slope (growth exponent) and R^2 of log(y) vs log(x). NaN if degenerate."""
    x = np.asarray(x, dtype=float)
    y = np.asarray(y, dtype=float)
    mask = (x > 0) & (y > 0)
    if mask.sum() < 2 or np.unique(x[mask]).size < 2:
        return float("nan"), float("nan")
    res = linregress(np.log(x[mask]), np.log(y[mask]))
    return res.slope, res.rvalue ** 2


# --------------------------------------------------------------------------
# E1 - comparisons & accesses vs n
# --------------------------------------------------------------------------
def analyze_e1(df):
    print("\n--- E1: Operation-count growth vs n ---")
    d = subset(df, "E1")
    if d.empty:
        print("  no E1 data.")
        return

    rows = []
    fig, ax = plt.subplots(figsize=(8, 6))
    for algo in [a for a in ALGO_ORDER if a in d["algorithm"].unique()]:
        s = d[d["algorithm"] == algo]
        for metric in ["comparisons", "accesses"]:
            g = s.groupby("n")[metric].mean().reset_index().dropna()
            if g.empty or g[metric].le(0).all():
                continue
            # naive/nth_element don't instrument accesses (naive uses std::sort,
            # logging a single trailing access) -- skip the meaningless ~0 exponent.
            if metric == "accesses" and g[metric].max() <= 1:
                continue
            exp, r2 = loglog_exponent(g["n"], g[metric])
            rows.append({"algorithm": algo, "metric": metric,
                         "growth_exponent": round(exp, 4), "r2": round(r2, 4)})
            if metric == "comparisons":
                ax.plot(g["n"], g[metric], marker="o", label=f"{algo}")
        # accesses/comparisons ratio (near-constant per algorithm, §E1); only
        # meaningful where accesses are actually instrumented.
        gc = s.groupby("n")["comparisons"].mean()
        ga = s.groupby("n")["accesses"].mean()
        if ga.max() > 1:
            ratio = (ga / gc).replace([np.inf, -np.inf], np.nan).dropna()
            if not ratio.empty:
                rows.append({"algorithm": algo, "metric": "accesses/comparisons",
                             "growth_exponent": round(ratio.mean(), 4), "r2": np.nan})

    ax.set_xscale("log"); ax.set_yscale("log")
    ax.set_xlabel("n"); ax.set_ylabel("comparisons (mean)")
    ax.set_title("E1: comparisons vs n (log-log)")
    ax.grid(True, which="both", ls=":"); ax.legend()
    save_fig(fig, "e1_comparisons_vs_n.png")
    save_table(pd.DataFrame(rows), "e1_growth_exponents.csv")


# --------------------------------------------------------------------------
# E2 / E8 - time models + crossover
# --------------------------------------------------------------------------
def _m_linear(n, a, b):  return a * n + b
def _m_nlogn(n, a, b):   return a * n * np.log(n) + b
def _m_quad(n, a, b):    return a * n * n + b

_MODELS = {"linear": _m_linear, "nlogn": _m_nlogn, "quadratic": _m_quad}


def analyze_e2_e8(df):
    print("\n--- E2 & E8: Time models and crossover ---")
    d = subset(df, "E2")
    if d.empty:
        print("  no E2 data.")
        return

    med = d.groupby(["algorithm", "n"])["time_ns"].median().reset_index()

    rows = []
    best = {}
    fig, ax = plt.subplots(figsize=(8, 6))
    for algo in [a for a in ALGO_ORDER if a in med["algorithm"].unique()]:
        s = med[med["algorithm"] == algo].sort_values("n")
        n = s["n"].to_numpy(float); t = s["time_ns"].to_numpy(float)
        ax.plot(n, t, marker="o", ls="none", label=f"{algo} (data)")
        if n.size < 3:
            continue
        best_r2, best_name, best_popt = -np.inf, None, None
        for name, fn in _MODELS.items():
            try:
                popt, _ = curve_fit(fn, n, t, maxfev=10000)
            except Exception:
                continue
            resid = t - fn(n, *popt)
            ss_res = np.sum(resid ** 2)
            ss_tot = np.sum((t - t.mean()) ** 2)
            r2 = 1 - ss_res / ss_tot if ss_tot > 0 else float("nan")
            rows.append({"algorithm": algo, "model": name,
                         "a": popt[0], "b": popt[1], "r2": round(r2, 5)})
            if r2 > best_r2:
                best_r2, best_name, best_popt = r2, name, popt
        if best_name is not None:
            best[algo] = (best_name, best_popt)
            ng = np.linspace(n.min(), n.max(), 200)
            ax.plot(ng, _MODELS[best_name](ng, *best_popt), ls="--",
                    label=f"{algo} fit={best_name}")

    ax.set_xlabel("n"); ax.set_ylabel("time (ns, median)")
    ax.set_title("E2/E8: time vs n with best-fit models")
    ax.grid(True, ls=":"); ax.legend()
    save_fig(fig, "e2_time_vs_n.png")
    save_table(pd.DataFrame(rows), "e2_time_models.csv")

    # E8: who is fastest at each n (regime table) + crossover brackets.
    pivot = med.pivot(index="n", columns="algorithm", values="time_ns").sort_index()
    regimes = []
    prev = None
    for n_val, row in pivot.iterrows():
        valid = row.dropna()
        if valid.empty:
            continue
        winner = valid.idxmin()
        regimes.append({"n": int(n_val), "fastest": winner,
                        "time_ns": round(valid.min(), 1)})
        prev = winner
    save_table(pd.DataFrame(regimes), "e8_regime_table.csv")
    if regimes:
        winners = [r["fastest"] for r in regimes]
        if len(set(winners)) == 1:
            print(f"  no crossover over sampled n: '{winners[0]}' fastest throughout.")
        else:
            # Report each crossover as the bracket [n_before, n_after] where the
            # fastest algorithm changes (no bootstrap CI -- the bracket is the
            # measurement-resolution-limited crossover location).
            cross = []
            for a, b in zip(regimes, regimes[1:]):
                if a["fastest"] != b["fastest"]:
                    print(f"  crossover: {a['fastest']} -> {b['fastest']} "
                          f"between n={a['n']} and n={b['n']}")
                    cross.append({"from": a["fastest"], "to": b["fastest"],
                                  "n_lo": a["n"], "n_hi": b["n"]})
            save_table(pd.DataFrame(cross), "e8_crossovers.csv")


# --------------------------------------------------------------------------
# E3 - quickselect runtime distribution
# --------------------------------------------------------------------------
def analyze_e3(df):
    print("\n--- E3: Quickselect runtime distribution ---")
    d = subset(df, "E3")
    if d.empty:
        print("  no E3 data.")
        return

    rows = []
    ns = sorted(d["n"].unique())
    for metric in ["comparisons", "time_ns"]:
        mu_list, sd_list, n_list = [], [], []
        for n_val in ns:
            v = d[d["n"] == n_val][metric].dropna().to_numpy(float)
            if v.size < 2:
                continue
            mu, sd = v.mean(), v.std(ddof=1)
            rows.append({"n": int(n_val), "metric": metric,
                         "mean": mu, "variance": sd ** 2, "std": sd,
                         "cv": sd / mu if mu else np.nan,
                         "skew": skew(v), "kurtosis": kurtosis(v),
                         "max": v.max(), "trials": v.size})
            mu_list.append(mu); sd_list.append(sd); n_list.append(n_val)
        # mean(n), variance(n), sigma(n) scaling via log-log fit (§E3: theory
        # predicts mean and sigma both ~linear in n, i.e. variance ~ n^2).
        if len(n_list) >= 2:
            e_mu, _ = loglog_exponent(n_list, mu_list)
            e_sd, _ = loglog_exponent(n_list, sd_list)
            var_list = [s ** 2 for s in sd_list]
            e_var, _ = loglog_exponent(n_list, var_list)
            print(f"  {metric}: mean(n)~n^{e_mu:.2f}, variance(n)~n^{e_var:.2f}, "
                  f"sigma(n)~n^{e_sd:.2f}")

    save_table(pd.DataFrame(rows), "e3_distribution_stats.csv")

    # histograms of comparisons per n (the tail is the object of study, no trimming)
    for n_val in ns:
        v = d[d["n"] == n_val]["comparisons"].dropna().to_numpy(float)
        if v.size < 5:
            continue
        fig, ax = plt.subplots(figsize=(7, 4))
        ax.hist(v, bins=40, color="steelblue", alpha=0.8)
        ax.axvline(v.mean(), color="red", ls="--", label=f"mean={v.mean():.0f}")
        ax.set_xlabel("comparisons"); ax.set_ylabel("count")
        ax.set_title(f"E3: quickselect comparisons, n={n_val} (M={v.size})")
        ax.legend()
        save_fig(fig, f"e3_hist_comparisons_n{n_val}.png")


# --------------------------------------------------------------------------
# E4 - k-dependency
# --------------------------------------------------------------------------
def _qs_expected_comparisons(n, kfrac):
    """2[n + k ln(n/k) + (n-k) ln(n/(n-k))], k=kfrac*n, safe at the extremes."""
    k = np.clip(kfrac, 1e-9, 1 - 1e-9) * n
    return 2.0 * (n + k * np.log(n / k) + (n - k) * np.log(n / (n - k)))


def analyze_e4(df):
    print("\n--- E4: k-dependency ---")
    d = subset(df, "E4")
    if d.empty:
        print("  no E4 data.")
        return

    g = d.groupby(["algorithm", "n", "k_frac"])["comparisons"].mean().reset_index()
    save_table(g, "e4_comparisons_by_kfrac.csv")

    fig, ax = plt.subplots(figsize=(8, 6))
    for algo in [a for a in ALGO_ORDER if a in g["algorithm"].unique()]:
        for n_val in sorted(g["n"].unique()):
            s = g[(g["algorithm"] == algo) & (g["n"] == n_val)].sort_values("k_frac")
            if s.empty:
                continue
            ax.plot(s["k_frac"], s["comparisons"], marker="o",
                    label=f"{algo} n={n_val}")

    # overlay the theoretical quickselect curve (scaled to the largest-n data)
    qs = g[g["algorithm"] == "quickselect"]
    if not qs.empty:
        n_big = qs["n"].max()
        s = qs[qs["n"] == n_big].sort_values("k_frac")
        kf = np.linspace(0.001, 0.999, 200)
        theo = _qs_expected_comparisons(n_big, kf)
        # scale so the theoretical median matches the empirical median
        scale = np.median(s["comparisons"]) / np.median(
            _qs_expected_comparisons(n_big, s["k_frac"].to_numpy()))
        ax.plot(kf, theo * scale, "k--", lw=2,
                label=f"theory x{scale:.2f} (n={n_big})")

    ax.set_xlabel("k / n"); ax.set_ylabel("comparisons (mean)")
    ax.set_title("E4: comparisons vs k/n")
    ax.grid(True, ls=":"); ax.legend(fontsize=8)
    save_fig(fig, "e4_k_dependency.png")


# --------------------------------------------------------------------------
# E5 - MoM group-size sweep
# --------------------------------------------------------------------------
def analyze_e5(df):
    print("\n--- E5: MoM group-size sweep ---")
    d = subset(df, "E5")
    if d.empty:
        print("  no E5 data.")
        return

    g = d.groupby("g").agg(
        comparisons=("comparisons", "mean"),
        accesses=("accesses", "mean"),
        time_ns=("time_ns", "median"),
        recursion_depth=("recursion_depth", "mean"),
    ).reset_index().sort_values("g")

    # THEORETICAL performance per group size (spec asks for theoretical AND
    # empirical). For odd g the median-of-medians guarantees ~n(g+1)/(4g)
    # elements on the pivot's small side, so the worst-case recursion side is
    # ~n(3g-1)/(4g); the median-of-medians subproblem is n/g. The algorithm is
    # provably linear iff 1/g + (3g-1)/(4g) < 1, i.e. g >= 5 (g=3 sums to 1).
    gv = g["g"].astype(float)
    g["theo_eliminated_frac"] = ((gv + 1.0) / (4.0 * gv)).round(4)
    g["theo_recursion_frac"] = ((3.0 * gv - 1.0) / (4.0 * gv)).round(4)
    g["theo_recurrence_sum"] = (1.0 / gv + (3.0 * gv - 1.0) / (4.0 * gv)).round(4)
    g["theo_linear_guarantee"] = g["theo_recurrence_sum"] < 1.0
    save_table(g, "e5_by_group_size.csv")

    g_star = int(g.loc[g["time_ns"].idxmin(), "g"])
    print(f"  empirically optimal g* (min median time) = {g_star}")
    print("  theoretical pruning (worst-case recursion fraction n*(3g-1)/(4g)):")
    for _, r in g.iterrows():
        print(f"    g={int(r['g']):>2}: eliminate>={r['theo_eliminated_frac']:.3f}n, "
              f"recurse<={r['theo_recursion_frac']:.3f}n, "
              f"sum={r['theo_recurrence_sum']:.3f} "
              f"-> {'LINEAR' if r['theo_linear_guarantee'] else 'NOT guaranteed linear'}")

    fig, axes = plt.subplots(1, 4, figsize=(19, 4))
    axes[0].plot(g["g"], g["comparisons"], marker="o"); axes[0].set_title("comparisons vs g (empirical)")
    axes[1].plot(g["g"], g["time_ns"], marker="o", color="darkorange"); axes[1].set_title("median time vs g (empirical)")
    axes[2].plot(g["g"], g["recursion_depth"], marker="o", color="green"); axes[2].set_title("recursion depth vs g (empirical)")
    axes[3].plot(g["g"], g["theo_recursion_frac"], marker="s", color="purple", label="recurse fraction")
    axes[3].axhline(0.75, ls="--", color="gray", label="limit 3/4 (g->inf)")
    axes[3].set_title("theoretical worst-case\nrecursion fraction vs g"); axes[3].legend(fontsize=8)
    for a in axes:
        a.set_xlabel("group size g"); a.grid(True, ls=":")
    fig.suptitle(f"E5: MoM vs g  (empirical g*={g_star}; theory: linear for g>=5)")
    save_fig(fig, "e5_group_size_sweep.png")


# --------------------------------------------------------------------------
# E6 - the g=3 anomaly
# --------------------------------------------------------------------------
def analyze_e6(df):
    print("\n--- E6: g=3 anomaly ---")
    d = subset(df, "E6")
    if d.empty:
        print("  no E6 data.")
        return

    rows = []
    fig, axes = plt.subplots(1, 2, figsize=(13, 5))
    for dist in sorted(d["distribution"].unique()):
        for gval in sorted(d["g"].unique()):
            s = d[(d["distribution"] == dist) & (d["g"] == gval)]
            gg = s.groupby("n").agg(comparisons=("comparisons", "mean"),
                                    recursion_depth=("recursion_depth", "mean")).reset_index()
            if gg.shape[0] < 2:
                continue
            slope, r2 = loglog_exponent(gg["n"], gg["comparisons"])
            rows.append({"distribution": dist, "g": int(gval),
                         "comparisons_exponent": round(slope, 4), "r2": round(r2, 4)})
            axes[0].plot(gg["n"], gg["comparisons"], marker="o", label=f"g={gval} {dist}")
            axes[1].plot(gg["n"], gg["recursion_depth"], marker="o", label=f"g={gval} {dist}")

    axes[0].set_xscale("log"); axes[0].set_yscale("log")
    axes[0].set_title("comparisons vs n (log-log)")
    axes[1].set_title("recursion depth vs n")
    for a in axes:
        a.set_xlabel("n"); a.grid(True, which="both", ls=":"); a.legend(fontsize=7)
    fig.suptitle("E6: g=3 vs g>=5  (linear needs 1/g + 3/4 < 1, i.e. g>=5)")
    save_fig(fig, "e6_g3_anomaly.png")
    rdf = pd.DataFrame(rows)
    save_table(rdf, "e6_slope_by_g.csv")

    # Explicit verdict: does g=3 drift above the g>=5 slopes? (the assignment's
    # "special attention" point). The g=3 degradation is a worst-case effect, so
    # on uniform/sorted input the drift may be small even at n=1e6 -- report
    # whichever the data shows rather than leaving bare exponents uninterpreted.
    if not rdf.empty:
        for dist in sorted(rdf["distribution"].unique()):
            sub = rdf[rdf["distribution"] == dist]
            s3 = sub[sub["g"] == 3]["comparisons_exponent"]
            s_hi = sub[sub["g"] >= 5]["comparisons_exponent"]
            if s3.empty or s_hi.empty:
                continue
            drift = float(s3.iloc[0]) - float(s_hi.mean())
            verdict = ("g=3 DRIFTS above g>=5 (anomaly visible)" if drift > 0.05
                       else "g=3 ~ g>=5 here: anomaly is worst-case, not triggered "
                            "by this input even at n<=1e6 (theory: 1/3+3/4>1)")
            print(f"  [{dist}] g=3 exp={float(s3.iloc[0]):.3f} vs "
                  f"mean(g>=5)={float(s_hi.mean()):.3f} (drift={drift:+.3f}) -> {verdict}")


# --------------------------------------------------------------------------
# E7 - robustness / adversarial
# --------------------------------------------------------------------------
def analyze_e7(df):
    print("\n--- E7: robustness / adversarial ---")
    d = subset(df, "E7")
    if d.empty:
        print("  no E7 data.")
        return

    g = d.groupby(["algorithm", "distribution", "n"])["comparisons"].mean().reset_index()
    save_table(g, "e7_comparisons_by_distribution.csv")

    # grouped bar at the largest n every algorithm reached
    n_big = g["n"].max()
    at_n = g[g["n"] == n_big]
    dists = sorted(at_n["distribution"].unique())
    algos = [a for a in ALGO_ORDER if a in at_n["algorithm"].unique()]
    x = np.arange(len(dists)); w = 0.8 / max(len(algos), 1)
    fig, ax = plt.subplots(figsize=(11, 6))
    for i, algo in enumerate(algos):
        vals = [at_n[(at_n["algorithm"] == algo) & (at_n["distribution"] == dd)]["comparisons"].mean()
                for dd in dists]
        ax.bar(x + i * w, vals, w, label=algo)
    ax.set_yscale("log")
    ax.set_xticks(x + w * (len(algos) - 1) / 2); ax.set_xticklabels(dists, rotation=30, ha="right")
    ax.set_ylabel("comparisons (mean, log)"); ax.set_title(f"E7: comparisons by distribution (n={n_big})")
    ax.legend(); ax.grid(True, axis="y", ls=":")
    save_fig(fig, "e7_by_distribution.png")

    # degradation curve on the all-equal stressor: comparisons vs n, log-log
    fig, ax = plt.subplots(figsize=(8, 6))
    rows = []
    for algo in algos:
        s = g[(g["algorithm"] == algo) & (g["distribution"] == "alleq")].sort_values("n")
        if s.shape[0] >= 2:
            slope, r2 = loglog_exponent(s["n"], s["comparisons"])
            rows.append({"algorithm": algo, "distribution": "alleq",
                         "comparisons_exponent": round(slope, 3), "r2": round(r2, 3)})
            ax.plot(s["n"], s["comparisons"], marker="o", label=f"{algo} (exp~{slope:.2f})")
    ax.set_xscale("log"); ax.set_yscale("log")
    ax.set_xlabel("n"); ax.set_ylabel("comparisons (mean)")
    ax.set_title("E7: degradation on ALLEQ (quickselect & naive-MoM go super-linear)")
    ax.grid(True, which="both", ls=":"); ax.legend()
    save_fig(fig, "e7_alleq_degradation.png")
    if rows:
        save_table(pd.DataFrame(rows), "e7_alleq_exponents.csv")


# --------------------------------------------------------------------------
# E9 - std::nth_element investigation
# --------------------------------------------------------------------------
def analyze_e9(df):
    print("\n--- E9: std::nth_element (introselect) ---")
    e7 = subset(df, "E7")
    e2 = subset(df, "E2")
    if e7.empty and e2.empty:
        print("  no E7/E2 data for E9.")
        return

    # comparisons across the E7 adversarial suite, nth_element vs the rest
    if not e7.empty:
        n_big = e7["n"].max()
        s = e7[e7["n"] == n_big]
        tab = s.groupby(["distribution", "algorithm"])["comparisons"].mean().unstack("algorithm")
        save_table(tab.reset_index(), "e9_comparisons_e7_suite.csv")

        dists = sorted(s["distribution"].unique())
        algos = [a for a in ALGO_ORDER if a in s["algorithm"].unique()]
        x = np.arange(len(dists)); w = 0.8 / max(len(algos), 1)
        fig, ax = plt.subplots(figsize=(11, 6))
        for i, algo in enumerate(algos):
            vals = [tab.loc[dd, algo] if (dd in tab.index and algo in tab.columns) else np.nan
                    for dd in dists]
            ax.bar(x + i * w, vals, w, label=algo)
        ax.set_yscale("log")
        ax.set_xticks(x + w * (len(algos) - 1) / 2); ax.set_xticklabels(dists, rotation=30, ha="right")
        ax.set_ylabel("comparisons (mean, log)")
        ax.set_title(f"E9: nth_element stays bounded where quickselect blows up (n={n_big})")
        ax.legend(); ax.grid(True, axis="y", ls=":")
        save_fig(fig, "e9_nth_element_vs_others.png")

    # time across the E2 size sweep
    if not e2.empty:
        med = e2.groupby(["algorithm", "n"])["time_ns"].median().reset_index()
        save_table(med.pivot(index="n", columns="algorithm", values="time_ns").reset_index(),
                   "e9_time_e2_sweep.csv")


# --------------------------------------------------------------------------
# main
# --------------------------------------------------------------------------
ANALYSES = {
    "E1": analyze_e1,
    "E2": analyze_e2_e8, "E8": analyze_e2_e8,
    "E3": analyze_e3,
    "E4": analyze_e4,
    "E5": analyze_e5,
    "E6": analyze_e6,
    "E7": analyze_e7,
    "E9": analyze_e9,
}


def parse_args():
    p = argparse.ArgumentParser(description="Analyze k-th element selection benchmarks.")
    p.add_argument("-e", "--experiment", default="ALL",
                   help="Experiment to analyze (E1..E9, or ALL). Default ALL.")
    return p.parse_args()


def main():
    args = parse_args()
    try:
        df = pd.read_csv(CSV_PATH)
    except FileNotFoundError:
        print(f"Error: {CSV_PATH} not found. Run experiments first (./run_all.sh).")
        return

    ensure_dirs()

    # data hygiene: flag any oracle failures (should never happen, §1.4)
    if "oracle_match" in df.columns:
        bad = df[df["oracle_match"].astype(str).str.lower() == "false"]
        if not bad.empty:
            print(f"WARNING: {len(bad)} rows with oracle_match=False -- results suspect!")

    if args.experiment == "ALL":
        # E8 shares analyze_e2_e8 with E2; call each distinct function once.
        funcs, seen = [], set()
        for exp in ["E1", "E2", "E3", "E4", "E5", "E6", "E7", "E9"]:
            fn = ANALYSES[exp]
            if fn not in seen:
                funcs.append(fn); seen.add(fn)
        to_run = funcs
    else:
        fn = ANALYSES.get(args.experiment)
        if fn is None:
            print(f"Unknown experiment '{args.experiment}'. Choose from {sorted(ANALYSES)} or ALL.")
            return
        to_run = [fn]

    for fn in to_run:
        try:
            fn(df)
        except Exception as e:  # one failing analysis must not abort the rest
            print(f"  !! {fn.__name__} failed: {type(e).__name__}: {e}")

    print(f"\nDone. Tables and figures in {OUT_DIR}/")


if __name__ == "__main__":
    main()
