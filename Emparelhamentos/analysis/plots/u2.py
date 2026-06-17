import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from analysis.load import load_combined_csv, filter_step_level
from analysis.aggregate import pivot_metrics
from analysis.config import apply_base_style, FIG_SIZE, DPI, FIG_EXT

# Color encodes alpha (edge density); marker encodes n
ALPHA_COLORS = {1.0: '#1f77b4', 1.5: '#ff7f0e', 2.0: '#2ca02c'}
N_MARKERS    = {200: 'o', 500: 's', 1000: '^', 2000: 'D'}


def run(data_dir, out_dir):
    print("[ANALYSIS] Running U2: Augmenting-path searches vs. defect...")
    try:
        df = load_combined_csv(data_dir, "U2")
    except FileNotFoundError:
        return

    df_step = filter_step_level(df)
    pivot_rep = pivot_metrics(df_step, ['n', 'alpha', 'algorithm', 'rep', 'iteration'])
    agg_df = pivot_rep.groupby(['n', 'alpha', 'algorithm', 'iteration']).mean().reset_index()

    available_pairs = sorted(set(zip(agg_df['n'], agg_df['alpha'])))
    max_phase = int(agg_df['iteration'].max())

    # Output 3: BFS edges / m per phase — all (n, alpha) on one chart
    fig3, ax3 = plt.subplots(figsize=FIG_SIZE)
    # Output 4: paths found / n per phase — same layout
    fig4, ax4 = plt.subplots(figsize=FIG_SIZE)

    for n, alpha in available_pairs:
        m      = round(n ** alpha)
        color  = ALPHA_COLORS.get(alpha, 'gray')
        marker = N_MARKERS.get(n, 'x')
        label  = f"n={n}, α={alpha}"

        mean_data = agg_df[(agg_df['n'] == n) & (agg_df['alpha'] == alpha)].sort_values('iteration')
        rep_data  = pivot_rep[(pivot_rep['n'] == n) & (pivot_rep['alpha'] == alpha)]

        ratio_mean = mean_data['bfs_edges_visited'] / m
        paths_mean = mean_data['paths_found_per_phase'] / n

        # Individual rep dots (faint background)
        for _, rep_grp in rep_data.groupby('rep'):
            rg = rep_grp.sort_values('iteration')
            ax3.scatter(rg['iteration'], rg['bfs_edges_visited'] / m,
                        color=color, alpha=0.15, s=18, zorder=1)
            ax4.scatter(rg['iteration'], rg['paths_found_per_phase'] / n,
                        color=color, alpha=0.15, s=18, zorder=1)

        # Mean line + markers
        ax3.plot(mean_data['iteration'], ratio_mean,
                 color=color, marker=marker, markersize=9, linewidth=1.5, label=label, zorder=3)
        ax4.plot(mean_data['iteration'], paths_mean,
                 color=color, marker=marker, markersize=9, linewidth=1.5, label=label, zorder=3)


    # Reference line: O(m) bound
    ax3.axhline(y=1.0, color='gray', linestyle='--', linewidth=1, label='O(m) bound')

    phase_labels = [f"Phase {i}" for i in range(max_phase + 1)]
    for ax in (ax3, ax4):
        ax.set_xticks(range(max_phase + 1))
        ax.set_xticklabels(phase_labels)
        ax.set_xlim(-0.3, max_phase + 0.3)

    apply_base_style(ax3, "BFS phase", "bfs_edges_visited / m",
                     "BFS work per phase (HK) — all graph sizes")
    fig3.tight_layout()
    fig3.savefig(os.path.join(out_dir, "figures", f"u2_bfs_per_phase.{FIG_EXT}"),
                 dpi=DPI, bbox_inches='tight')
    plt.close(fig3)

    apply_base_style(ax4, "BFS phase", "paths_found_per_phase / n",
                     "Matching progress per phase (HK) — all graph sizes")
    fig4.tight_layout()
    fig4.savefig(os.path.join(out_dir, "figures", f"u2_paths_per_phase.{FIG_EXT}"),
                 dpi=DPI, bbox_inches='tight')
    plt.close(fig4)

    print("      -> U2 complete. Internal search profiles exported.")
