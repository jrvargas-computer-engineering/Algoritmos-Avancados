# analysis/plots/c2.py
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level
from analysis.fit import fit_power_law
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

def run(data_dir, out_dir):
    print("[ANALYSIS] Running C2: Full weighted algorithm head-to-head...")
    try:
        df = load_combined_csv(data_dir, "C2")
    except FileNotFoundError:
        print("      -> C2.csv not found. Skipping.")
        return

    df_run = filter_run_level(df)

    # Output 20: Scalability Table
    df_time = df_run[df_run['metric_name'] == 'wall_clock_ms']
    agg_time = df_time.groupby(['n', 'alpha', 'algorithm'])['metric_value'].agg(['mean', 'std']).reset_index()

    pivot_time = agg_time.pivot(index=['n', 'alpha'], columns='algorithm')
    pivot_time.columns = [f"{'BF' if 'BellmanFord' in col[1] else 'JD'}_{col[0]}_ms" for col in pivot_time.columns]
    pivot_time = pivot_time.reset_index()

    if 'BF_mean_ms' in pivot_time.columns and 'JD_mean_ms' in pivot_time.columns:
        pivot_time['speedup_ratio'] = pivot_time['BF_mean_ms'] / pivot_time['JD_mean_ms']
    
    pivot_time.to_csv(os.path.join(out_dir, "c2_scalability_table.csv"), index=False)

    # Output 20b: Operations Table
    df_ops = df_run[df_run['metric_name'].isin(['total_edge_relaxations', 'total_dijkstra_calls'])]
    agg_ops = df_ops.groupby(['n', 'alpha', 'algorithm', 'metric_name'])['metric_value'].mean().unstack(level=['algorithm', 'metric_name']).reset_index()
    
    # Safely flatten multi-index for ops table
    ops_cols = ['n', 'alpha']
    for col in agg_ops.columns[2:]:
        prefix = 'BF' if 'BellmanFord' in col[0] else 'JD'
        suffix = 'relaxations' if 'relaxations' in col[1] else 'dijkstra_calls'
        ops_cols.append(f"{prefix}_mean_{suffix}")
    
    agg_ops.columns = ops_cols
    agg_ops.to_csv(os.path.join(out_dir, "c2_operations_table.csv"), index=False)

    # Output 21: Fitted Exponent Comparison
    exponents = []
    alphas = sorted(df_time['alpha'].unique())
    for alpha in alphas:
        subset = agg_time[agg_time['alpha'] == alpha]
        for algo in subset['algorithm'].unique():
            algo_data = subset[subset['algorithm'] == algo].sort_values('n')
            k_emp, _, r_sq = fit_power_law(algo_data['n'], algo_data['mean'])
            
            is_bf = "BellmanFord" in algo
            k_theory = (2.0 + alpha) if is_bf else (1.0 + alpha)
            delta_k = k_emp - k_theory if pd.notnull(k_emp) else np.nan
            
            exponents.append({
                'alpha': alpha, 'Algorithm': algo, 'k_theory': k_theory,
                'k_emp': round(k_emp, 2) if pd.notnull(k_emp) else np.nan,
                'delta_k': round(delta_k, 2) if pd.notnull(delta_k) else np.nan,
                'R2': round(r_sq, 4) if pd.notnull(r_sq) else np.nan,
                'note': '' if is_bf else 'log n excluded from fit',
                'flag': 'REVIEW' if pd.notnull(delta_k) and abs(delta_k) > 0.20 else 'OK'
            })
            
    pd.DataFrame(exponents).to_csv(os.path.join(out_dir, "c2_exponent_table.csv"), index=False)

    # Output 22: Large-n Ratio Validation
    if 2.0 in alphas and 'speedup_ratio' in pivot_time.columns:
        subset_a2 = pivot_time[pivot_time['alpha'] == 2.0].sort_values('n')
        subset_a2 = subset_a2[subset_a2['n'] > 1] # Avoid log2(1) == 0
        
        subset_a2['predicted_speedup'] = subset_a2['n'] / np.log2(subset_a2['n'])
        
        fig, ax = plt.subplots(figsize=FIG_SIZE)
        ax.plot(subset_a2['n'], subset_a2['speedup_ratio'], marker=STYLES['HungarianMatcher+JohnsonDijkstraStrategy']['marker'], color=STYLES['HungarianMatcher+JohnsonDijkstraStrategy']['color'], label='Empirical Speedup (BF/JD)')
        ax.plot(subset_a2['n'], subset_a2['predicted_speedup'], linestyle='--', color='gray', label='n / log₂(n)')
        
        ax.set_xscale('log')
        apply_base_style(ax, "n (log scale)", "Speedup ratio", "Large-n Speedup Validation (α = 2.0)")
        
        # Annotate highest n
        last_row = subset_a2.iloc[-1]
        pct = (last_row['speedup_ratio'] / last_row['predicted_speedup']) * 100
        ax.annotate(f"{pct:.1f}% of theory", (last_row['n'], last_row['speedup_ratio']), textcoords="offset points", xytext=(-40, 10))
        
        fig.savefig(os.path.join(out_dir, "figures", f"c2_large_n_ratio.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    # C2 Publication Plot — one figure per alpha
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
        apply_base_style(ax, "n", "Mean wall-clock time (ms)", f"α = {alpha}")
        plt.tight_layout()
        fig.savefig(os.path.join(out_dir, "figures", f"c2_scalability_plot_a{alpha}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    print("      -> C2 complete. Weighted Master Tables generated.")