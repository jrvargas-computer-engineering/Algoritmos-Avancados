import os
import pandas as pd
import numpy as np
from analysis.load import load_combined_csv, filter_run_level
from analysis.correctness import enforce_correctness, print_summary

def run(data_dir, out_dir):
    print("[ANALYSIS] Running Output 0a: Unweighted Oracle Correctness (U0)...")
    try:
        df = load_combined_csv(data_dir, "U0")
    except FileNotFoundError:
        return

    df_run = filter_run_level(df)
    
    # 1. Isolate the exact Oracle truth baselines
    oracle_df = df_run[(df_run['algorithm'] == 'Oracle') & (df_run['metric_name'] == 'oracle_size')]
    oracle_map = oracle_df.set_index(['n', 'alpha', 'rep'])['metric_value'].to_dict()
    
    # 2. Isolate the algorithmic executions
    algo_df = df_run[(df_run['algorithm'] != 'Oracle') & (df_run['metric_name'] == 'matching_size')].copy()
    if algo_df.empty:
        return
        
    algo_df = algo_df.rename(columns={'metric_value': 'returned_size'})
    
    # 3. Broadcast the oracle baseline to the algorithmic runs based on execution coordinates
    algo_df['oracle_size'] = algo_df.apply(lambda row: oracle_map.get((row['n'], row['alpha'], row['rep']), np.nan), axis=1)
    
    algo_df['correct'] = algo_df['returned_size'] == algo_df['oracle_size']
    
    final_cols = ['n', 'alpha', 'algorithm', 'rep', 'returned_size', 'oracle_size', 'correct']
    final_df = algo_df[final_cols]
    
    out_path = os.path.join(out_dir, "u0_oracle_correctness.csv")
    final_df.to_csv(out_path, index=False)
    
    print_summary(final_df, 'algorithm', 'correct')
    enforce_correctness(final_df, 'correct', 'U0', "One or more unweighted algorithms failed to match the exact oracle cardinality.")
    print("      -> Output 0a complete. Unweighted Oracle Gate Passed.\n")