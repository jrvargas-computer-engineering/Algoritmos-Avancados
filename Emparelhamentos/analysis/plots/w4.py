import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level
from analysis.aggregate import agg_mean_std
from analysis.correctness import enforce_correctness
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

def run(data_dir, out_dir):
    print("[ANALYSIS] Running W4: Negative-weight stress tests...")
    try:
        df = load_combined_csv(data_dir, "W4")
    except FileNotFoundError:
        return

    df_run = filter_run_level(df)

    # Output 17b: Potential Violations Across Regimes
    df_pot = df_run[(df_run['metric_name'] == 'potential_violations_count') & 
                    (df_run['algorithm'].str.contains('JohnsonDijkstra'))].copy()
    if not df_pot.empty:
        df_pot['pass'] = df_pot['metric_value'] == 0
        df_pot[['n', 'weight_regime', 'rep', 'metric_value', 'pass']].to_csv(os.path.join(out_dir, "w4_potential_violations.csv"), index=False)
        enforce_correctness(df_pot, 'pass', 'W4 Output 17b', "JD generated negative transformed edge weights under extreme weight regimes.")

    # Output 17: Matching Value Correctness Across Regimes
    df_val = df_run[df_run['metric_name'] == 'matching_value']
    pivot_val = df_val.pivot_table(index=['n', 'weight_regime', 'rep'], columns='algorithm', values='metric_value').reset_index()
    pivot_val.columns = [f"val_{'BF' if 'BellmanFord' in col else 'JD'}" if col in STYLES else col for col in pivot_val.columns]
    
    pivot_val['abs_diff'] = abs(pivot_val['val_BF'] - pivot_val['val_JD'])
    pivot_val['pass'] = pivot_val['abs_diff'] < 1e-6
    pivot_val.to_csv(os.path.join(out_dir, "w4_correctness.csv"), index=False)
    enforce_correctness(pivot_val, 'pass', 'W4 Output 17', "BF and JD maximum matching weights diverged under extreme weight regimes.")

    # Output 16: Grouped Bar Chart by Regime
    df_time = df_run[df_run['metric_name'] == 'wall_clock_ms']
    agg_time = agg_mean_std(df_time, ['n', 'weight_regime', 'algorithm'])
    ns = sorted(agg_time['n'].unique())
    regimes = ['mixed', 'negative', 'positive']
    
    x = np.arange(len(regimes))
    width = 0.35

    for n in ns:
        subset = agg_time[agg_time['n'] == n]
        fig, ax = plt.subplots(figsize=FIG_SIZE)

        bf_means, bf_stds, jd_means, jd_stds = [], [], [], []
        for reg in regimes:
            reg_data = subset[subset['weight_regime'] == reg]

            bf_row = reg_data[reg_data['algorithm'].str.contains('BellmanFord')]
            bf_means.append(bf_row['mean'].values[0] if not bf_row.empty else 0)
            bf_stds.append(bf_row['std'].values[0] if not bf_row.empty else 0)

            jd_row = reg_data[reg_data['algorithm'].str.contains('JohnsonDijkstra')]
            jd_means.append(jd_row['mean'].values[0] if not jd_row.empty else 0)
            jd_stds.append(jd_row['std'].values[0] if not jd_row.empty else 0)

        ax.bar(x - width/2, bf_means, width, yerr=bf_stds, label='BF',
               color=STYLES['HungarianMatcher+BellmanFordStrategy']['color'], capsize=3)
        ax.bar(x + width/2, jd_means, width, yerr=jd_stds, label='JD',
               color=STYLES['HungarianMatcher+JohnsonDijkstraStrategy']['color'], capsize=3)

        ax.set_xticks(x)
        ax.set_xticklabels(regimes)
        apply_base_style(ax, "Weight regime", "Mean wall-clock time (ms)", f"n = {n}")
        plt.tight_layout()
        fig.savefig(os.path.join(out_dir, "figures", f"w4_regime_times_n{n}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()
    
    print("      -> W4 complete. Edge constraint bounds survived extreme mapping.")