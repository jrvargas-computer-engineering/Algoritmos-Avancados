import os
import pandas as pd
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level
from analysis.aggregate import agg_mean_std
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

def run(data_dir, out_dir):
    print("[ANALYSIS] Running U4: Sensitivity to edge density...")
    try:
        df = load_combined_csv(data_dir, "U4")
    except FileNotFoundError:
        return

    df_run = filter_run_level(df)
    
    # Output 7: Wall-Clock vs Alpha
    df_time = df_run[df_run['metric_name'] == 'wall_clock_ms']
    agg_time = agg_mean_std(df_time, ['n', 'alpha', 'algorithm'])
    ns = sorted(agg_time['n'].unique())

    for n in ns:
        subset = agg_time[agg_time['n'] == n].sort_values('alpha')
        fig, ax = plt.subplots(figsize=FIG_SIZE)

        for algo in subset['algorithm'].unique():
            algo_data = subset[subset['algorithm'] == algo]
            style = STYLES.get(algo, {})

            ax.plot(algo_data['alpha'], algo_data['mean'], color=style.get('color'),
                    linestyle=style.get('linestyle'), marker=style.get('marker'), label=style.get('label'))
            ax.fill_between(algo_data['alpha'], algo_data['mean'] - algo_data['std'],
                            algo_data['mean'] + algo_data['std'], color=style.get('color'), alpha=0.2)

        apply_base_style(ax, "α (edge-density exponent)", "Mean wall-clock time (ms)", f"n = {n}")
        plt.tight_layout()
        fig.savefig(os.path.join(out_dir, "figures", f"u4_wall_clock_vs_alpha_n{n}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    # Output 8: HK Phase Count vs Alpha
    df_phases = df_run[(df_run['metric_name'] == 'phase_count') & (df_run['algorithm'] == 'HopcroftKarpMatcher')]
    
    if not df_phases.empty:
        agg_phases = agg_mean_std(df_phases, ['n', 'alpha'])
        fig, ax = plt.subplots(figsize=FIG_SIZE)
        
        for n in sorted(agg_phases['n'].unique()):
            subset = agg_phases[agg_phases['n'] == n].sort_values('alpha')
            ax.plot(subset['alpha'], subset['mean'], marker='s', label=f"n = {n}")
            
        apply_base_style(ax, "α", "Mean phase count (HK)", "Phase Count Independence")
        fig.savefig(os.path.join(out_dir, "figures", f"u4_phase_count_vs_alpha.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    print("      -> U4 complete. Density limits established.")