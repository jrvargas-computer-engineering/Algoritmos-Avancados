# analysis/correctness.py
import sys
import pandas as pd

def enforce_correctness(df, bool_column, test_id, error_message):
    """
    Evaluates a boolean correctness column. If any row is False,
    prints the failing rows to stderr and immediately halts execution.
    """
    failures = df[~df[bool_column]]
    
    total_runs = len(df)
    passed_runs = total_runs - len(failures)
    
    print(f"      [{test_id}] Verification: {passed_runs}/{total_runs} passed.")
    
    if not failures.empty:
        sys.stderr.write(f"\n[FATAL ERROR] {test_id} Correctness Gate Failed!\n")
        sys.stderr.write(f"{error_message}\n")
        sys.stderr.write("Failing Instances:\n")
        sys.stderr.write(failures.to_string(index=False) + "\n")
        sys.stderr.write("\nABORTING DOWNSTREAM ANALYSIS.\n")
        sys.exit(1)

def print_summary(df, algorithm_col, bool_col):
    """Prints a summary line per algorithm (total correct / total rows)."""
    summary = df.groupby(algorithm_col)[bool_col].agg(['sum', 'count']).reset_index()
    for _, row in summary.iterrows():
        print(f"        -> {row[algorithm_col]}: {row['sum']}/{row['count']} correct")