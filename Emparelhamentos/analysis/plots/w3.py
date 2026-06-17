import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level
from analysis.aggregate import agg_median_p90, pivot_metrics
from analysis.fit import fit_power_law
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

def run(data_dir, out_dir):
    print("[ANALYSIS] Running W3: Wall-clock scalability (BF vs JD)...")
    try:
        df = load_combined_csv(data_dir, "W3")
    except FileNotFoundError:
        return

    df_run = filter_run_level(df)
    
    # Outputs 13 & 14 (Identical mechanism to U3)
    df_time = df_run[df_run['metric_name'] == 'wall_clock_ms']
    agg_df = agg_median_p90(df_time, ['n', 'alpha', 'algorithm'])
    alphas = sorted(agg_df['alpha'].unique())
    
    exponents = []

    for alpha in alphas:
        subset = agg_df[agg_df['alpha'] == alpha]
        fig, ax = plt.subplots(figsize=FIG_SIZE)

        for algo in subset['algorithm'].unique():
            algo_data = subset[subset['algorithm'] == algo].sort_values('n')
            style = STYLES.get(algo, {})
            ax.plot(algo_data['n'], algo_data['median'], color=style.get('color'),
                    linestyle=style.get('linestyle'), marker=style.get('marker'), label=style.get('label'))
            ax.fill_between(algo_data['n'], algo_data['median'], algo_data['p90'],
                            color=style.get('color'), alpha=0.2)

            k_emp, A, r_sq = fit_power_law(algo_data['n'], algo_data['median'])
            k_theory = (2.0 + alpha) if "BellmanFord" in algo else (1.0 + alpha)
            exponents.append({
                'alpha': alpha, 'Algorithm': algo, 'k_theory': k_theory,
                'k_emp': round(k_emp, 2) if pd.notnull(k_emp) else np.nan,
                'R2': round(r_sq, 4) if pd.notnull(r_sq) else np.nan
            })

        ax.set_xscale('log')
        ax.set_yscale('log')
        apply_base_style(ax, "n", "Wall-clock time (ms), median", f"α = {alpha}")
        plt.tight_layout()
        fig.savefig(os.path.join(out_dir, "figures", f"w3_wall_clock_a{alpha}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    pd.DataFrame(exponents).to_csv(os.path.join(out_dir, "w3_fitted_exponents.csv"), index=False)

    # Output 14: BF/JD Speedup Heatmap
    pivot_df = pivot_metrics(agg_df, ['n', 'alpha'], columns='algorithm', values='median')
    bf_col = 'HungarianMatcher+BellmanFordStrategy'
    jd_col = 'HungarianMatcher+JohnsonDijkstraStrategy'
    if bf_col in pivot_df.columns and jd_col in pivot_df.columns:
        pivot_df['speedup_ratio'] = pivot_df[bf_col] / pivot_df[jd_col]
        heatmap_data = pivot_df.pivot(index='alpha', columns='n', values='speedup_ratio')

        fig, ax = plt.subplots(figsize=FIG_SIZE)
        cax = ax.imshow(heatmap_data.values, origin='lower', cmap='YlOrRd', aspect='auto')
        plt.colorbar(cax, label="Speedup (BF / JD)")

        ax.set_xticks(np.arange(len(heatmap_data.columns)))
        ax.set_yticks(np.arange(len(heatmap_data.index)))
        ax.set_xticklabels(heatmap_data.columns)
        ax.set_yticklabels(heatmap_data.index)
        ax.set_xlabel("n (vertices per side)")
        ax.set_ylabel("α (edge density)")
        ax.set_title("Empirical speedup ratio (BF / JD)")
        ax.grid(False)

        vmax = np.nanmax(heatmap_data.values)
        for i in range(len(heatmap_data.index)):
            for j in range(len(heatmap_data.columns)):
                val = heatmap_data.values[i, j]
                if not np.isnan(val):
                    text_color = "white" if val > vmax * 0.6 else "black"
                    ax.text(j, i, f"{val:.1f}", ha="center", va="center", color=text_color, fontsize=8)
                    if val >= 1.0:
                        ax.add_patch(plt.Rectangle((j - 0.5, i - 0.5), 1, 1,
                                                   fill=False, edgecolor='black', linewidth=2))

        fig.savefig(os.path.join(out_dir, "figures", f"w3_speedup_heatmap.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    # Output 15: JD Time Decomposition (Stacked Bar)
    df_jd = df_run[df_run['algorithm'].str.contains('JohnsonDijkstra')]
    if not df_jd.empty:
        pivot_jd = pivot_metrics(df_jd, ['n', 'alpha', 'rep'])
        if 'wall_clock_ms' in pivot_jd.columns and 'potential_update_overhead_ms' in pivot_jd.columns:
            pivot_jd['dijkstra_ms'] = pivot_jd['wall_clock_ms'] - pivot_jd['potential_update_overhead_ms']
            agg_jd = pivot_jd.groupby(['n', 'alpha'])[['dijkstra_ms', 'potential_update_overhead_ms']].mean().reset_index()
            
            # Restrict to requested alpha=1.5
            target_jd = agg_jd[agg_jd['alpha'] == 1.5].sort_values('n')
            
            fig, ax = plt.subplots(figsize=FIG_SIZE)
            ns_str = target_jd['n'].astype(str)
            ax.bar(ns_str, target_jd['dijkstra_ms'], color="#0F6E56", label="Dijkstra Pathfinding")
            ax.bar(ns_str, target_jd['potential_update_overhead_ms'], bottom=target_jd['dijkstra_ms'], color="#854F0B", label="Potential Maintenance Overhead")
            
            # Annotate overhead fraction
            for i, row in target_jd.iterrows():
                total = row['dijkstra_ms'] + row['potential_update_overhead_ms']
                pct = (row['potential_update_overhead_ms'] / total) * 100 if total > 0 else 0
                ax.text(str(row['n']), total, f"{pct:.1f}%", ha='center', va='bottom', fontsize=9)
                
            apply_base_style(ax, "n", "Time (ms)", "JD Execution Decomposition (α = 1.5)")
            fig.savefig(os.path.join(out_dir, "figures", f"w3_jd_decomposition.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
            plt.close()

    print("      -> W3 complete. Speedup boundaries established.")