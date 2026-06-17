# analysis/load.py
import pandas as pd
import os

SCHEMA = {
    'test_id': str,
    'n': int,
    'alpha': float,
    'rep': int,
    'algorithm': str,
    'iteration': int,
    'weight_regime': str,
    'metric_name': str,
    'metric_value': float
}

def load_combined_csv(data_dir, test_id):
    """
    Loads the aggregated CSV for a specific test (e.g., 'U1.csv').
    Validates against the universal 9-column schema.
    """
    file_path = os.path.join(data_dir, f"{test_id}.csv")
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Combined data file not found: {file_path}")
    
    df = pd.read_csv(file_path, dtype=SCHEMA)
    return df

def filter_run_level(df):
    """Filters data for run-level scalars (iteration == -1)."""
    return df[df['iteration'] == -1].copy()

def filter_step_level(df):
    """Filters data for per-step time series (iteration >= 0)."""
    return df[df['iteration'] >= 0].copy()