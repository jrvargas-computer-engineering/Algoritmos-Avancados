import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from analysis.load import load_combined_csv, filter_run_level, filter_step_level
from analysis.correctness import enforce_correctness
from analysis.config import STYLES, apply_base_style, FIG_SIZE, DPI, FIG_EXT

MAX_TRACE_POINTS = 100  # downsample dense iteration traces to this many points

def run(data_dir, out_dir):
    print("[ANALYSIS] Running W1: Main-loop defect reduction...")
    try:
        df = load_combined_csv(data_dir, "W1")
    except FileNotFoundError:
        return

    # Output 9: Correctness Assertion Table
    df_run = filter_run_level(df)
    df_metrics = df_run[df_run['metric_name'].isin(['augmentation_count', 'matching_value'])]
    
    pivot = df_metrics.pivot_table(
        index=['n', 'alpha', 'rep'], 
        columns=['metric_name', 'algorithm'], 
        values='metric_value'
    ).reset_index()

    # Flatten MultiIndex columns
    pivot.columns = [f"{col[0]}_{'BF' if 'BellmanFord' in col[1] else 'JD'}" if col[1] else col[0] for col in pivot.columns]
    
    pivot['aug_match'] = pivot['augmentation_count_BF'] == pivot['augmentation_count_JD']
    pivot['val_match'] = abs(pivot['matching_value_BF'] - pivot['matching_value_JD']) < 1e-6
    pivot['pass'] = pivot['aug_match'] & pivot['val_match']

    out_path = os.path.join(out_dir, "w1_correctness.csv")
    pivot.to_csv(out_path, index=False)
    
    enforce_correctness(pivot, 'pass', 'W1 Output 9', "BF and JD diverged in augmentation count or final optimal value.")

    # Output 10: Defect vs Iteration Traces
    df_step = filter_step_level(df)
    df_defect = df_step[df_step['metric_name'] == 'defect_at_iteration']
    
    agg_defect = df_defect.groupby(['n', 'alpha', 'algorithm', 'iteration'])['metric_value'].mean().reset_index()
    
    target_ns = [100, 500, 1000]
    target_alphas = [1.0, 2.0]
    available_pairs = set(zip(agg_defect['n'], agg_defect['alpha']))

    for alpha in target_alphas:
        for n in target_ns:
            if (n, alpha) not in available_pairs:
                continue

            subset = agg_defect[(agg_defect['n'] == n) & (agg_defect['alpha'] == alpha)]
            fig, ax = plt.subplots(figsize=FIG_SIZE)

            for algo in subset['algorithm'].unique():
                algo_data = subset[subset['algorithm'] == algo].sort_values('iteration')
                style = STYLES.get(algo, {})

                # Downsample dense traces; markers are omitted — the line conveys the shape
                if len(algo_data) > MAX_TRACE_POINTS:
                    idx = np.linspace(0, len(algo_data) - 1, MAX_TRACE_POINTS, dtype=int)
                    algo_data = algo_data.iloc[idx]

                ax.plot(algo_data['iteration'], algo_data['metric_value'],
                        color=style.get('color'), linestyle=style.get('linestyle'),
                        linewidth=1.5, label=style.get('label'), alpha=0.85)

            apply_base_style(ax, "Augmentation step", "Defect (unmatched vertices)", f"n={n}, α={alpha}")
            plt.tight_layout()
            fig.savefig(os.path.join(out_dir, "figures", f"w1_defect_vs_iteration_a{alpha}_n{n}.{FIG_EXT}"), dpi=DPI, bbox_inches='tight')
            plt.close()
    print("      -> W1 complete. Parity validated and traces plotted.")