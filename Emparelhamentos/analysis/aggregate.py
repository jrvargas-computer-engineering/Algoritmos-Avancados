# analysis/aggregate.py
import pandas as pd
import numpy as np

def agg_mean_std(df, group_cols, value_col='metric_value'):
    """Computes mean and std over repetitions."""
    return df.groupby(group_cols)[value_col].agg(
        mean='mean', 
        std='std'
    ).reset_index()

def agg_median_p90(df, group_cols, value_col='metric_value'):
    """Computes median and 90th percentile over repetitions (used in U3, W3)."""
    return df.groupby(group_cols)[value_col].agg(
        median='median',
        p90=lambda x: np.percentile(x, 90)
    ).reset_index()

def pivot_metrics(df, index_cols, columns='metric_name', values='metric_value'):
    """
    Pivots multiple metric names into separate columns for a single run/iteration.
    Useful for cross-referencing values (e.g. wall_clock_ms and overhead_ms).
    """
    return df.pivot_table(
        index=index_cols, 
        columns=columns, 
        values=values, 
        aggfunc='first'
    ).reset_index()