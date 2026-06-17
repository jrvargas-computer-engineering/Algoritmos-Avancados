import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level
from analysis.aggregate import agg_mean_std
from analysis.fit import fit_power_law
from analysis.correctness import enforce_correctness
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

def run(data_dir, out_dir):
    print("[ANALYSIS] Running U1: Main-loop iteration count vs. graph size...")
    try:
        df = load_combined_csv(data_dir, "U1")
    except FileNotFoundError:
        print("      -> U1.csv not found. Skipping.")
        return

    df_run = filter_run_level(df)

    # Output 2: Correctness Gate (Cross-Validation)
    df_size = df_run[df_run['metric_name'] == 'matching_size'].copy()
    if not df_size.empty:
        pivot_size = df_size.pivot_table(
            index=['n', 'alpha', 'rep'],
            columns='algorithm',
            values='metric_value'
        ).reset_index()

        if 'HopcroftKarpMatcher' in pivot_size.columns and 'SimpleAugmentingMatcher' in pivot_size.columns:
            pivot_size['pass'] = pivot_size['HopcroftKarpMatcher'] == pivot_size['SimpleAugmentingMatcher']
            enforce_correctness(
                pivot_size,
                'pass',
                'U1 Output 2',
                "Hopcroft-Karp and Simple Augmenting Matcher diverged on maximum matching size."
            )

    # Output 1: Main Loop Iterations Plot & Fit
    # HK records phase_count as main_loop_iters (BFS phases, O(√n)).
    # Simple does not record main_loop_iters; each augmentation adds one match,
    # so matching_size equals the number of main-loop iterations (O(n)).
    df_iters_hk = df_run[df_run['metric_name'] == 'main_loop_iters']

    df_iters_simple = df_run[
        (df_run['metric_name'] == 'matching_size') &
        (df_run['algorithm'] == 'SimpleAugmentingMatcher')
    ].copy()
    df_iters_simple['metric_name'] = 'main_loop_iters'

    df_iters = pd.concat([df_iters_hk, df_iters_simple], ignore_index=True)
    agg_iters = agg_mean_std(df_iters, ['n', 'alpha', 'algorithm'])

    alphas = sorted(agg_iters['alpha'].unique())
    exponents = []

    for alpha in alphas:
        subset = agg_iters[agg_iters['alpha'] == alpha]
        fig, ax = plt.subplots(figsize=FIG_SIZE)

        n_range = np.array(sorted(subset['n'].unique()), dtype=float)

        for algo in subset['algorithm'].unique():
            algo_data = subset[subset['algorithm'] == algo].sort_values('n')
            style = STYLES.get(algo, {})

            ax.errorbar(algo_data['n'], algo_data['mean'], yerr=algo_data['std'],
                        color=style.get('color'), linestyle=style.get('linestyle'),
                        marker=style.get('marker'), label=style.get('label'), capsize=3)

            k_emp, A, r_sq = fit_power_law(algo_data['n'], algo_data['mean'])
            k_theory = 1.0 if "Simple" in algo else 0.5

            exponents.append({
                'alpha': alpha, 'Algorithm': algo, 'k_theory': k_theory,
                'k_emp': round(k_emp, 2) if pd.notnull(k_emp) else np.nan,
                'R2': round(r_sq, 4) if pd.notnull(r_sq) else np.nan
            })

            # Theoretical reference line fitted to data range
            if pd.notnull(A) and pd.notnull(k_theory):
                ref_y = A * (n_range ** k_theory)
                ax.plot(n_range, ref_y, color=style.get('color'), linestyle=':', linewidth=1,
                        alpha=0.6, label=f"{style.get('label', algo)} theory (k={k_theory})")

            if not pd.isnull(k_emp):
                last_pt = algo_data.iloc[-1]
                ax.annotate(f"k={k_emp:.2f}", (last_pt['n'], last_pt['mean']),
                            textcoords="offset points", xytext=(10, -5), color=style.get('color'))

        ax.set_xscale('log')
        ax.set_yscale('log')
        apply_base_style(ax, "n (vertices per side)", "Main-loop iterations", f"α = {alpha}")
        plt.tight_layout()
        fig.savefig(os.path.join(out_dir, "figures", f"u1_main_loop_iters_a{alpha}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    pd.DataFrame(exponents).to_csv(os.path.join(out_dir, "u1_fitted_exponents.csv"), index=False)
    print("      -> U1 complete. Plot and exponents exported.")
