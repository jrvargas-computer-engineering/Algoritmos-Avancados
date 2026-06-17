import os
import pandas as pd
import numpy as np
from analysis.load import load_combined_csv, filter_run_level
from analysis.correctness import enforce_correctness, print_summary

def run(data_dir, out_dir):
    print("[ANALYSIS] Running Output 0b: Weighted Oracle Correctness (W0)...")
    try:
        df = load_combined_csv(data_dir, "W0")
    except FileNotFoundError:
        return

    df_run = filter_run_level(df)
    
    # 1. Isolate the exact Oracle truth baselines
    oracle_df = df_run[df_run['algorithm'] == 'Oracle']
    oracle_sizes = oracle_df[oracle_df['metric_name'] == 'oracle_size'].set_index(['n', 'alpha', 'rep'])['metric_value'].to_dict()
    oracle_vals = oracle_df[oracle_df['metric_name'] == 'oracle_value'].set_index(['n', 'alpha', 'rep'])['metric_value'].to_dict()
    
    # 2. Isolate the algorithmic executions
    algo_df = df_run[df_run['algorithm'] != 'Oracle']
    if algo_df.empty:
        return
        
    pivot_df = algo_df.pivot_table(
        index=['n', 'alpha', 'algorithm', 'rep'],
        columns='metric_name',
        values='metric_value',
        aggfunc='first'
    ).reset_index()
    
    pivot_df = pivot_df.rename(columns={
        'matching_size': 'ret_size',
        'matching_value': 'ret_value'
    })
    
    # 3. Broadcast the oracle baseline to the algorithmic runs based on execution coordinates
    pivot_df['ora_size'] = pivot_df.apply(lambda row: oracle_sizes.get((row['n'], row['alpha'], row['rep']), np.nan), axis=1)
    pivot_df['ora_value'] = pivot_df.apply(lambda row: oracle_vals.get((row['n'], row['alpha'], row['rep']), np.nan), axis=1)
    
    pivot_df['size_correct'] = pivot_df['ret_size'] == pivot_df['ora_size']
    pivot_df['value_correct'] = abs(pivot_df['ret_value'] - pivot_df['ora_value']) < 1e-6
    pivot_df['fully_correct'] = pivot_df['size_correct'] & pivot_df['value_correct']
    
    final_cols = ['n', 'alpha', 'algorithm', 'rep', 'ret_size', 'ora_size', 
                  'size_correct', 'ret_value', 'ora_value', 'value_correct']
    final_df = pivot_df[final_cols]
    
    out_path = os.path.join(out_dir, "w0_oracle_correctness.csv")
    final_df.to_csv(out_path, index=False)
    
    print_summary(pivot_df, 'algorithm', 'fully_correct')
    enforce_correctness(pivot_df, 'fully_correct', 'W0', "One or more weighted algorithms failed cardinality or maximum weight parity.")
    print("      -> Output 0b complete. Weighted Oracle Gate Passed.\n")