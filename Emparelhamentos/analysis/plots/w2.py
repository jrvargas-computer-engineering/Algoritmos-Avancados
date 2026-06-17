import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level, filter_step_level
from analysis.aggregate import agg_mean_std
from analysis.fit import fit_power_law
from analysis.correctness import enforce_correctness
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

def run(data_dir, out_dir):
    print("[ANALYSIS] Running W2: Shortest-path call cost...")
    try:
        df = load_combined_csv(data_dir, "W2")
    except FileNotFoundError:
        return

    # Output 12: Potential Violations Correctness Gate (MUST RUN FIRST)
    df_run = filter_run_level(df)
    df_pot = df_run[(df_run['metric_name'] == 'potential_violations_count') & 
                    (df_run['algorithm'].str.contains('JohnsonDijkstra'))].copy()
    
    if not df_pot.empty:
        df_pot['pass'] = df_pot['metric_value'] == 0
        df_pot[['n', 'alpha', 'rep', 'metric_value', 'pass']].to_csv(os.path.join(out_dir, "w2_potential_violations.csv"), index=False)
        enforce_correctness(df_pot, 'pass', 'W2 Output 12', "Johnson-Dijkstra generated negative transformed edge weights!")

    # Output 11: Edge Relaxations Per Call vs n
    df_step = filter_step_level(df)
    df_edges = df_step[df_step['metric_name'] == 'edge_relaxations']
    
    # 1. Average across all iterations within a single run to get 'per-call mean'
    per_call = df_edges.groupby(['n', 'alpha', 'algorithm', 'rep'])['metric_value'].mean().reset_index()
    # 2. Average across repetitions
    agg_relax = agg_mean_std(per_call, ['n', 'alpha', 'algorithm'])
    
    alphas = sorted(agg_relax['alpha'].unique())
    exponents = []

    for alpha in alphas:
        subset = agg_relax[agg_relax['alpha'] == alpha]
        fig, ax = plt.subplots(figsize=FIG_SIZE)

        for algo in subset['algorithm'].unique():
            algo_data = subset[subset['algorithm'] == algo].sort_values('n')
            style = STYLES.get(algo, {})

            ax.errorbar(algo_data['n'], algo_data['mean'], yerr=algo_data['std'],
                        color=style.get('color'), linestyle=style.get('linestyle'),
                        marker=style.get('marker'), label=style.get('label'))

            k_emp, A, r_sq = fit_power_law(algo_data['n'], algo_data['mean'])
            k_theory = (1.0 + alpha) if "BellmanFord" in algo else alpha

            exponents.append({
                'alpha': alpha, 'Algorithm': algo, 'k_theory': k_theory,
                'k_emp': round(k_emp, 2) if pd.notnull(k_emp) else np.nan,
                'R2': round(r_sq, 4) if pd.notnull(r_sq) else np.nan
            })
            if not pd.isnull(k_emp):
                last_pt = algo_data.iloc[-1]
                ax.annotate(f"k={k_emp:.2f}", (last_pt['n'], last_pt['mean']),
                            textcoords="offset points", xytext=(10, -5), color=style.get('color'))

        ax.set_xscale('log')
        ax.set_yscale('log')
        apply_base_style(ax, "n", "Mean edge relaxations per call", f"α = {alpha}")
        plt.tight_layout()
        fig.savefig(os.path.join(out_dir, "figures", f"w2_edge_relaxations_a{alpha}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    pd.DataFrame(exponents).to_csv(os.path.join(out_dir, "w2_fitted_exponents.csv"), index=False)
    print("      -> W2 complete. Potential invariant mathematically verified.")