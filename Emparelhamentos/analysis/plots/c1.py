# analysis/plots/c1.py
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level
from analysis.fit import fit_power_law
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

def run(data_dir, out_dir):
    print("[ANALYSIS] Running C1: Full unweighted algorithm head-to-head...")
    try:
        df = load_combined_csv(data_dir, "C1")
    except FileNotFoundError:
        print("      -> C1.csv not found. Skipping.")
        return

    df_run = filter_run_level(df)

    # Output 18: Scalability Table
    df_time = df_run[df_run['metric_name'] == 'wall_clock_ms']
    agg_time = df_time.groupby(['n', 'alpha', 'algorithm'])['metric_value'].agg(
        mean='mean', std='std', min='min', max='max'
    ).reset_index()

    pivot_time = agg_time.pivot(index=['n', 'alpha'], columns='algorithm')
    pivot_time.columns = [f"{'Simple' if 'Simple' in col[1] else 'HK'}_{col[0]}_ms" for col in pivot_time.columns]
    pivot_time = pivot_time.reset_index()

    if 'Simple_mean_ms' in pivot_time.columns and 'HK_mean_ms' in pivot_time.columns:
        pivot_time['speedup_ratio'] = pivot_time['Simple_mean_ms'] / pivot_time['HK_mean_ms']
    
    pivot_time.to_csv(os.path.join(out_dir, "c1_scalability_table.csv"), index=False)

    # Output 18b: Operations Companion Table
    df_ops = df_run[df_run['metric_name'].isin(['main_loop_iters', 'total_edges_visited'])]
    agg_ops = df_ops.groupby(['n', 'alpha', 'algorithm', 'metric_name'])['metric_value'].mean().unstack(level=['algorithm', 'metric_name']).reset_index()
    
    ops_cols = ['n', 'alpha']
    for col in agg_ops.columns[2:]:
        prefix = 'Simple' if 'Simple' in col[0] else 'HK'
        suffix = 'iters' if 'iters' in col[1] else 'edges'
        ops_cols.append(f"{prefix}_mean_{suffix}")
    
    agg_ops.columns = ops_cols
    agg_ops.to_csv(os.path.join(out_dir, "c1_operations_table.csv"), index=False)

    # Output 19: Fitted Exponent Comparison
    exponents = []
    alphas = sorted(df_time['alpha'].unique())
    for alpha in alphas:
        subset = agg_time[agg_time['alpha'] == alpha]
        for algo in subset['algorithm'].unique():
            algo_data = subset[subset['algorithm'] == algo].sort_values('n')
            k_emp, _, r_sq = fit_power_law(algo_data['n'], algo_data['mean'])
            k_theory = (1.0 + alpha) if "Simple" in algo else (0.5 + alpha)
            delta_k = k_emp - k_theory if pd.notnull(k_emp) else np.nan
            
            exponents.append({
                'alpha': alpha, 'Algorithm': algo, 'k_theory': k_theory,
                'k_emp': round(k_emp, 2) if pd.notnull(k_emp) else np.nan,
                'delta_k': round(delta_k, 2) if pd.notnull(delta_k) else np.nan,
                'R2': round(r_sq, 4) if pd.notnull(r_sq) else np.nan,
                'flag': 'REVIEW' if pd.notnull(delta_k) and abs(delta_k) > 0.15 else 'OK'
            })
            
    pd.DataFrame(exponents).to_csv(os.path.join(out_dir, "c1_exponent_table.csv"), index=False)

    # Output 18 Figure: Publication Scalability Plot — one figure per alpha
    for alpha in alphas:
        subset = agg_time[agg_time['alpha'] == alpha]
        fig, ax = plt.subplots(figsize=FIG_SIZE)
        for algo in subset['algorithm'].unique():
            algo_data = subset[subset['algorithm'] == algo].sort_values('n')
            style = STYLES.get(algo, {})
            ax.plot(algo_data['n'], algo_data['mean'], color=style.get('color'),
                    linestyle=style.get('linestyle'), marker=style.get('marker'), label=style.get('label'))
            ax.fill_between(algo_data['n'], algo_data['mean'] - algo_data['std'],
                            algo_data['mean'] + algo_data['std'], color=style.get('color'), alpha=0.2)
        ax.set_xscale('log')
        ax.set_yscale('log')
        apply_base_style(ax, "n (vertices per side)", "Mean wall-clock time (ms)", f"α = {alpha}")
        plt.tight_layout()
        fig.savefig(os.path.join(out_dir, "figures", f"c1_scalability_plot_a{alpha}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    print("      -> C1 complete. Unweighted Master Tables generated.")