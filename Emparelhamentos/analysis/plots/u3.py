import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level
from analysis.aggregate import agg_median_p90, pivot_metrics
from analysis.fit import fit_power_law
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

def run(data_dir, out_dir):
    print("[ANALYSIS] Running U3: Wall-clock scalability (Simple vs HK)...")
    try:
        df = load_combined_csv(data_dir, "U3")
    except FileNotFoundError:
        return

    df_time = filter_run_level(df)
    df_time = df_time[df_time['metric_name'] == 'wall_clock_ms']
    
    agg_df = agg_median_p90(df_time, ['n', 'alpha', 'algorithm'])
    alphas = sorted(agg_df['alpha'].unique())
    
    # Output 5: Wall-Clock Log-Log — one figure per alpha
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
            k_theory = (1.0 + alpha) if "Simple" in algo else (0.5 + alpha)

            exponents.append({
                'alpha': alpha, 'Algorithm': algo, 'k_theory': k_theory,
                'k_emp': round(k_emp, 2) if pd.notnull(k_emp) else np.nan,
                'R2': round(r_sq, 4) if pd.notnull(r_sq) else np.nan
            })

        ax.set_xscale('log')
        ax.set_yscale('log')
        apply_base_style(ax, "n (vertices per side)", "Wall-clock time (ms), median", f"α = {alpha}")
        plt.tight_layout()
        fig.savefig(os.path.join(out_dir, "figures", f"u3_wall_clock_a{alpha}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()

    pd.DataFrame(exponents).to_csv(os.path.join(out_dir, "u3_fitted_exponents.csv"), index=False)

    # Output 6: Speedup Heatmap
    pivot_df = pivot_metrics(agg_df, ['n', 'alpha'], columns='algorithm', values='median')
    if "SimpleAugmentingMatcher" in pivot_df.columns and "HopcroftKarpMatcher" in pivot_df.columns:
        pivot_df['speedup_ratio'] = pivot_df['SimpleAugmentingMatcher'] / pivot_df['HopcroftKarpMatcher']

        heatmap_data = pivot_df.pivot(index='alpha', columns='n', values='speedup_ratio')

        fig, ax = plt.subplots(figsize=FIG_SIZE)
        cax = ax.imshow(heatmap_data.values, origin='lower', cmap='YlOrRd', aspect='auto')
        plt.colorbar(cax, label="Speedup (Simple / HK)")

        ax.set_xticks(np.arange(len(heatmap_data.columns)))
        ax.set_yticks(np.arange(len(heatmap_data.index)))
        ax.set_xticklabels(heatmap_data.columns)
        ax.set_yticklabels(heatmap_data.index)
        ax.set_xlabel("n (vertices per side)")
        ax.set_ylabel("α (edge density)")
        ax.set_title("Empirical speedup ratio (Simple / HK)")
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

        fig.savefig(os.path.join(out_dir, "figures", f"u3_speedup_heatmap.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
        plt.close()
    
    print("      -> U3 complete. Cross-over boundaries analyzed.")