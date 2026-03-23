import os
import sys
import argparse
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import matplotlib.patches as mpatches

# ==========================================
# COMMON HELPER FUNCTIONS
# ==========================================

def load_and_prep_data(filepath):
    """Loads the CSV and adds a replication ID since runs are repeated."""
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found.")
        sys.exit(1)
        
    df = pd.read_csv(filepath)
    
    # FIXED: Replaced the typo with the correct Python 'not in' syntax
    if 'rep' not in df.columns:
        df['rep'] = df.groupby(['k', 'n', 'm']).cumcount()
        
    return df

def setup_directories():
    """Ensures the output directory for plots exists."""
    os.makedirs("plots", exist_ok=True)

# ==========================================
# SUITE A: SPECIFIC GRAPH FUNCTIONS
# ==========================================
def plot_algo_performance_vs_k(df):
    """
    Graph 1: Algorithm Performance vs. Heap Degree k (Faceted Subplots)
    FIX: Fully independent Y-scales for each M value.
    """
    n_values = df['n'].unique()
    
    for n in n_values:
        df_n = df[df['n'] == n]
        if df_n.empty:
            continue
            
        k_order = sorted(df_n['k'].unique())
        
        # 1. Create FacetGrid with sharey=False
        g = sns.FacetGrid(df_n, col="m", height=5, aspect=1.2, 
                          sharey=False, col_wrap=3)
        
        # 2. Map Boxplots (Boxplot will now define the local Y-limit)
        g.map_dataframe(
            sns.boxplot, 
            x='k', 
            y='Algo_Time_ms', 
            order=k_order,
            color='skyblue',
            linewidth=1.2,
            fliersize=4,
            boxprops=dict(alpha=0.6)
        )
        
        # 3. Map Pointplot (The trend line)
        g.map_dataframe(
            sns.pointplot, 
            x='k', 
            y='Algo_Time_ms', 
            order=k_order,
            estimator='median', 
            errorbar=None,
            color='red',
            markers='o',
            scale=0.7
        )
        
        # 4. Formatting
        g.set_axis_labels("Heap Degree (k)", "Algorithm Time (ms)")
        g.set_titles(col_template="Edges: m = {col_name}")
        
        plt.subplots_adjust(top=0.85)
        g.fig.suptitle(f"Dijkstra Performance vs k (n={n})\n(Independent Y-Scales per Panel)", fontsize=14)
        
        # --- CRITICAL FIX FOR INDEPENDENT SCALING ---
        for ax in g.axes.flat:
            ax.set_ylim(bottom=0) # Only fix the bottom at 0
            ax.yaxis.set_major_locator(plt.MaxNLocator(nbins=10)) # Ensure enough ticks
            ax.autoscale(enable=True, axis='y') # Force individual zoom
            ax.grid(True, axis='y', linestyle='--', alpha=0.4)
            
        filename = f"plots/suiteA_graph1_performance_n{n}.png"
        plt.savefig(filename, bbox_inches='tight')
        plt.close()
        print(f"Created: {filename}")

import math

def plot_heap_complexity(df):
    """
    Graph 2: Heap Complexity Validation (Combined Grid)
    Plots the mean fractions with shaded standard deviation bands for all k values
    in a single, publication-ready grid (2 plots per row).
    """
    k_values = sorted(df['k'].unique())
    if not k_values:
        return
        
    cols = 2
    # Calculate how many rows are needed to fit all k values
    rows = math.ceil(len(k_values) / cols)
    
    # Create the grid. sharey=True ensures fair visual comparison across all k
    fig, axes = plt.subplots(rows, cols, figsize=(14, 5 * rows), sharey=True)
    
    # Flatten the axes array to easily iterate over it
    if isinstance(axes, plt.Axes):
        axes = [axes]
    else:
        axes = axes.flatten()
        
    for i, k in enumerate(k_values):
        ax = axes[i]
        df_k = df[df['k'] == k]
        
        if df_k.empty:
            ax.set_visible(False)
            continue
            
        grouped = df_k.groupby('alpha')[['r_up', 'r_down']].agg(['mean', 'std']).reset_index()
        grouped = grouped.sort_values('alpha')
        grouped.fillna(0, inplace=True)
        
        # Plot r_up
        ax.plot(grouped['alpha'], grouped['r_up']['mean'], marker='s', color='blue', label='Mean r_up')
        ax.fill_between(grouped['alpha'], 
                         grouped['r_up']['mean'] - grouped['r_up']['std'],
                         grouped['r_up']['mean'] + grouped['r_up']['std'],
                         color='blue', alpha=0.15)
                         
        # Plot r_down
        ax.plot(grouped['alpha'], grouped['r_down']['mean'], marker='^', color='red', label='Mean r_down')
        ax.fill_between(grouped['alpha'], 
                         grouped['r_down']['mean'] - grouped['r_down']['std'],
                         grouped['r_down']['mean'] + grouped['r_down']['std'],
                         color='red', alpha=0.15)
        
        # Theoretical boundary
        ax.axhline(y=1.0, color='black', linestyle='--', label='Theoretical Max (1.0)')
        
        ax.set_title(f"Heap Degree: k = {k}", fontsize=12, fontweight='bold')
        ax.set_xlabel("Graph Density (alpha = log_n m)")
        
        # Only add the Y-label to the leftmost plots to keep the grid clean
        if i % cols == 0:
            ax.set_ylabel("Fraction of Sift Operations (r)")
            
        ax.set_ylim(0.0, 1.2)
        ax.grid(True, linestyle='--', alpha=0.6)
        
        # Only add the legend to the very first subplot so it doesn't clutter the rest
        if i == 0:
            ax.legend(loc='upper right')
            
    # If there is an odd number of k values (like 5), hide the final empty subplot box
    for j in range(len(k_values), len(axes)):
        axes[j].set_visible(False)
        
    # Main title for the entire figure
    plt.suptitle("Heap Complexity Validation Across Different Degrees (k)\n(Solid line = Mean | Shaded area = ±1 Std Dev)", fontsize=16, y=1.02)
    plt.tight_layout()
    
    filename = "plots/suiteA_graph2_heap_complexity_combined.png"
    plt.savefig(filename, bbox_inches='tight')
    plt.close()
    print(f"Created: {filename}")
    
def plot_update_complexity(df):
    """
    Graph 3: Algorithm Update Complexity (u)
    Plots the mean update fraction u with a shaded variance band.
    """
    # Group by n and alpha, calculating mean and std for 'u'
    grouped = df.groupby(['n', 'alpha'])['u'].agg(['mean', 'std']).reset_index()
    grouped.fillna(0, inplace=True)
    
    plt.figure(figsize=(10, 6))
    n_values = sorted(grouped['n'].unique())
    colors = ['teal', 'magenta']
    
    for i, n in enumerate(n_values):
        df_n = grouped[grouped['n'] == n].sort_values('alpha')
        color = colors[i % len(colors)]
        
        # Plot mean line
        plt.plot(df_n['alpha'], df_n['mean'], marker='o', color=color, label=f'n = {n} (Mean)')
        
        # Plot shaded variance
        plt.fill_between(df_n['alpha'],
                         df_n['mean'] - df_n['std'],
                         df_n['mean'] + df_n['std'],
                         color=color, alpha=0.15)
        
    plt.title("Algorithm Update Complexity\n(Solid line = Mean | Shaded area = ±1 Std Dev)")
    plt.xlabel("Graph Density (alpha = log_n m)")
    plt.ylabel("Update Fraction (u = update / m)")
    plt.ylim(bottom=0)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend()
    
    filename = "plots/suiteA_graph3_update_complexity.png"
    plt.savefig(filename, bbox_inches='tight')
    plt.close()
    print(f"Created: {filename}")


def plot_mean_case_analysis(df):
    """
    Graph 4: Mean-Case Analysis vs Noshita's Expected Value
    Line plot showing empirical updates vs Noshita's theoretical expectation.
    Exports the mean and standard deviation data to a CSV table.
    """
    target_n = df['n'].max() 
    df_plot = df[df['n'] == target_n].copy()
    
    if df_plot.empty:
        print(f"Warning: No data found for n={target_n}. Skipping Graph 4.")
        return

    # Calculate mean and std deviation for empirical updates
    grouped = df_plot.groupby('m').agg(
        emp_mean=('update', 'mean'),
        emp_std=('update', 'std'),
        noshita=('noshita_expected', 'first') 
    ).reset_index()
    
    # Sort by m to ensure logical progression
    grouped = grouped.sort_values('m')
    
    # ---------------------------------------------------------
    # EXPORT DATA TO CSV TABLE
    # ---------------------------------------------------------
    # Format the numbers to 2 decimal places for a cleaner report
    table_df = grouped.copy()
    table_df['emp_mean'] = table_df['emp_mean'].round(2)
    table_df['emp_std'] = table_df['emp_std'].round(2)
    table_df['noshita'] = table_df['noshita'].round(2)
    
    # Save to the reports folder
    table_filename = f"reports/suiteA_graph4_mean_case_data_n{target_n}.csv"
    table_df.to_csv(table_filename, index=False)
    print(f"Created Table: {table_filename}")

    # ---------------------------------------------------------
    # GENERATE PLOT
    # ---------------------------------------------------------
    plt.figure(figsize=(10, 6))
    
    # 1. Plot the Empirical Data (Mean line ONLY)
    plt.plot(grouped['m'], grouped['emp_mean'], marker='o', color='royalblue', 
             linewidth=2, label='Empirical Updates (Mean)')
    
    # 2. Plot Noshita's Theoretical Expected Values
    plt.plot(grouped['m'], grouped['noshita'], marker='D', color='red', 
             linestyle='--', linewidth=2, label="Noshita Expected (Theoretical)")
    
    plt.title(f"Mean-Case Analysis vs Noshita Expected Value (n={target_n})\n(Empirical Growth vs Theoretical Ceiling)")    
    plt.xlabel("Number of Edges (m)")
    plt.ylabel("Number of Update Operations")
    
    # Ensure the Y-axis starts at 0 
    plt.ylim(bottom=0)
    plt.grid(True, linestyle='--', alpha=0.6)
    
    plt.legend(loc='upper left')
    
    filename = f"plots/suiteA_graph4_mean_case_n{target_n}.png"
    plt.savefig(filename, bbox_inches='tight')
    plt.close()
    print(f"Created Plot: {filename}")
# ==========================================
# SUITES B & C: TIME COMPLEXITY PLOTS
# ==========================================

def verify_operations(df, suite_name):
    """Verifies that operations stay within theoretical limits."""
    invalid_i = df[df['insert'] > df['n']]
    invalid_d = df[df['deletemin'] > df['n']]
    invalid_u = df[df['update'] > df['m']]
    
    print(f"\n--- Operation Bounds Verification ({suite_name}) ---")
    print(f"Inserts > n   : {len(invalid_i)} violations")
    print(f"Deletemins > n: {len(invalid_d)} violations")
    print(f"Updates > m   : {len(invalid_u)} violations")
    
    if len(invalid_i) == 0 and len(invalid_d) == 0 and len(invalid_u) == 0:
        print("SUCCESS: All theoretical operation bounds are perfectly respected!")
    else:
        print("WARNING: Some operations exceeded theoretical limits.")
    print("--------------------------------------------------\n")

def plot_normalized_time_complexity(df, varying_var, fixed_var, fixed_val, suite_name):
    """
    Plots T / ((n+m) * log(n)) as a function of the varying variable (m or n).
    Uses Seaborn's lineplot to naturally display the mean and standard deviation.
    """
    # Calculate Normalized Time: T / ((n+m) * log2(n))
    df['T_norm'] = df['Algo_Time_ms'] / ((df['n'] + df['m']) * np.log2(df['n']))
    
    plt.figure(figsize=(10, 6))
    
    # Plot line with shaded standard deviation error band
    sns.lineplot(data=df, x=varying_var, y='T_norm', marker='o', color='indigo', errorbar='sd', linewidth=2)
    
    plt.title(f"Normalized Time Complexity: T / ((n+m)log n)\n(Fixed {fixed_var} = {fixed_val})")
    plt.xlabel(f"Number of {'Edges (m)' if varying_var == 'm' else 'Vertices (n)'}")
    plt.ylabel("Normalized Time")
    
    # Use logarithmic scale on X-axis since the parameters scale exponentially (2^i or sqrt(2)^i)
    plt.xscale('log', base=2) 
    plt.grid(True, linestyle='--', alpha=0.6)
    
    filename = f"plots/suite{suite_name}_normalized_complexity_varying_{varying_var}.png"
    plt.savefig(filename, bbox_inches='tight')
    plt.close()
    print(f"Created: {filename}")


# ==========================================
# REGRESSION ANALYSIS (SUITES A & B)
# ==========================================

def perform_multiple_linear_regression(file_a, file_b):
    """
    Performs T(n,m) ~ a * n^b * m^c by applying log to both sides:
    log(T) = log(a) + b*log(n) + c*log(m).
    Now also generates Actual vs Predicted and Error plots.
    """
    print("\n=== MULTIPLE LINEAR REGRESSION ANALYSIS ===")
    
    # Load and combine data
    try:
        df_a = pd.read_csv(file_a)
        df_b = pd.read_csv(file_b)
        df = pd.concat([df_a, df_b], ignore_index=True)
    except FileNotFoundError:
        print(f"Error: Missing files for regression. Ensure {file_a} and {file_b} exist.")
        return
        
    # We drop extremely fast runs (e.g. < 1ms) because system timer noise ruins log regressions
    df = df[df['Algo_Time_ms'] > 1.0].copy()
    
    # Convert to log scale
    log_T = np.log2(df['Algo_Time_ms'])
    log_n = np.log2(df['n'])
    log_m = np.log2(df['m'])
    
    # Prepare matrix X for least squares: [1, log_n, log_m]
    X = np.column_stack([np.ones(len(df)), log_n, log_m])
    
    # Perform Least Squares Regression
    coefficients, residuals, rank, s = np.linalg.lstsq(X, log_T, rcond=None)
    
    # Extract coefficients
    log_a, b, c = coefficients
    a = 2 ** log_a
    
    print("Regression Equation Formula:")
    print("T(n,m) ≈ a * n^b * m^c\n")
    print(f"Calculated Parameters:")
    print(f"  a = {a:.6e} (Constant)")
    print(f"  b = {b:.4f} (Power of n)")
    print(f"  c = {c:.4f} (Power of m)")
    print("===========================================\n")

    # ---------------------------------------------------------
    # PLOTTING THE REGRESSION RESULTS
    # ---------------------------------------------------------
    setup_directories()
    
    # Calculate the Predicted Time using our newly found coefficients
    df['Predicted_Time_ms'] = a * (df['n'] ** b) * (df['m'] ** c)
    
    # Plot 1: Actual vs Predicted Scatter Plot (Log-Log Scale)
    plt.figure(figsize=(8, 8))
    plt.scatter(df['Algo_Time_ms'], df['Predicted_Time_ms'], alpha=0.6, color='royalblue', edgecolor='k')
    
    # Draw the ideal y=x line (where Actual == Predicted)
    max_val = max(df['Algo_Time_ms'].max(), df['Predicted_Time_ms'].max())
    min_val = min(df['Algo_Time_ms'].min(), df['Predicted_Time_ms'].min())
    plt.plot([min_val, max_val], [min_val, max_val], color='red', linestyle='--', linewidth=2, label='Perfect Fit (y = x)')
    
    # Use Log scale because our times range from a few milliseconds to thousands of milliseconds
    plt.xscale('log')
    plt.yscale('log')
    
    plt.title(f"Multiple Linear Regression Fit\nModel: T ≈ ({a:.2e}) * n^{b:.2f} * m^{c:.2f}")
    plt.xlabel("Actual Algorithm Time (ms)")
    plt.ylabel("Predicted Algorithm Time (ms)")
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.6)
    
    filename_fit = "plots/suiteR_actual_vs_predicted.png"
    plt.savefig(filename_fit, bbox_inches='tight')
    plt.close()
    print(f"Created: {filename_fit}")
    
    # Plot 2: Relative Error Distribution
    plt.figure(figsize=(8, 6))
    
    # Calculate relative error percentage: (Predicted - Actual) / Actual * 100
    df['Error_Pct'] = ((df['Predicted_Time_ms'] - df['Algo_Time_ms']) / df['Algo_Time_ms']) * 100
    
    # Plot a histogram with a Kernel Density Estimate (KDE) curve
    sns.histplot(df['Error_Pct'], bins=30, kde=True, color='purple', edgecolor='black')
    plt.axvline(x=0, color='red', linestyle='--', linewidth=2, label='Zero Error')
    
    plt.title("Regression Residuals (Relative Error %)\n(Centered around 0 means highly accurate)")
    plt.xlabel("Error Percentage (%)")
    plt.ylabel("Frequency of Runs")
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.6)
    
    filename_err = "plots/suiteR_residuals_error.png"
    plt.savefig(filename_err, bbox_inches='tight')
    plt.close()
    print(f"Created: {filename_err}")
    print("Suite R plotting complete.\n")

# ==========================================
# SUITE D: REAL-WORLD DIMACS PLOTS
# ==========================================
def plot_suite_D_summary(file_perf, file_report):
    """
    Generates Time Profiling, Memory Usage, and Operation Counts 
    for massive real-world DIMACS graphs using optimal visual representations.
    """
    print("\nGenerating charts for Test Suite D...")
    try:
        df_perf = pd.read_csv(file_perf)
        df_report = pd.read_csv(file_report)
    except FileNotFoundError:
        print(f"Error: Missing files. Ensure {file_perf} and {file_report} exist.")
        return

    setup_directories()
    graph_name = df_report['Graph'].iloc[0].split('/')[-1]
    
    # ---------------------------------------------------------
    # 1. IO Time vs Algo Time (Bar Chart with Error Bars)
    # ---------------------------------------------------------
    plt.figure(figsize=(8, 6))
    
    # Calculate means and standard deviations (converted to seconds)
    io_mean = df_perf['IO_Time'].mean() / 1000
    io_std = df_perf['IO_Time'].std() / 1000
    algo_mean = df_perf['Algo_Time'].mean() / 1000
    algo_std = df_perf['Algo_Time'].std() / 1000
    
    labels = ['I/O Time\n(Reading Graph)', 'Algorithm Time\n(Dijkstra Execution)']
    means = [io_mean, algo_mean]
    stds = [io_std, algo_std]
    
    # Plotting the bars
    bars = plt.bar(labels, means, yerr=stds, capsize=8, color=['#ff9999', '#66b3ff'], edgecolor='black', alpha=0.8)
    
    # Add text labels on top of the bars for exact precision
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval + 1, f"{yval:.1f}s", ha='center', va='bottom', fontweight='bold')

    plt.title(f"Time Profiling on Real Graph ({graph_name})\n(Average Time ± Std Dev)")
    plt.ylabel("Time (seconds)")
    plt.grid(axis='y', linestyle='--', alpha=0.6)
    
    filename_time = f"plots/suiteD_time_profile_{graph_name}.png"
    plt.savefig(filename_time, bbox_inches='tight')
    plt.close()
    print(f"Created: {filename_time}")


    # ---------------------------------------------------------
    # 2. Memory Usage Stability (Line Plot over Runs)
    # ---------------------------------------------------------
    plt.figure(figsize=(8, 5))
    
    # Line plot shows stability across sequential runs
    sns.lineplot(data=df_report, x='Run_ID', y='Memory_MB', marker='o', color='forestgreen', linewidth=2)
    
    plt.title(f"Memory Consumption Stability ({graph_name})\n(Flat line proves no memory leaks across 30 runs)")
    plt.xlabel("Execution Run ID")
    plt.ylabel("Memory (MB)")
    plt.grid(True, linestyle='--', alpha=0.6)
    
    # Anchor at 0 to show true scale of RAM usage, give 10% headroom
    plt.ylim(0, df_report['Memory_MB'].max() * 1.1)
    
    filename_mem = f"plots/suiteD_memory_{graph_name}.png"
    plt.savefig(filename_mem, bbox_inches='tight')
    plt.close()
    print(f"Created: {filename_mem}")


    # ---------------------------------------------------------
    # 3. Operations breakdown (Horizontal Bar Chart with Log Scale)
    # ---------------------------------------------------------
    plt.figure(figsize=(10, 6))
    
    ops_mean = df_perf[['I', 'D', 'U', 'Sift_Up', 'Sift_Down']].mean()
    ops_std = df_perf[['I', 'D', 'U', 'Sift_Up', 'Sift_Down']].std()
    
    labels = ['Insert (I)', 'Deletemin (D)', 'Update (U)', 'Sift Up', 'Sift Down']
    
    # Horizontal bar charts are much better for reading category labels and long bars
    plt.barh(labels, ops_mean, xerr=ops_std, capsize=5, color='coral', edgecolor='black', alpha=0.8)
    
    plt.title(f"Operation Counts Breakdown ({graph_name})\n(Note: Logarithmic Scale)")
    plt.xlabel("Number of Operations (Log10 Base)")
    
    # Log scale on X-axis since Sift Down is massive
    plt.xscale('log')
    plt.grid(axis='x', linestyle='--', alpha=0.6)
    
    filename_ops = f"plots/suiteD_operations_{graph_name}.png"
    plt.savefig(filename_ops, bbox_inches='tight')
    plt.close()
    print(f"Created: {filename_ops}")
    print("Suite D plotting complete.\n")


# ==========================================
# ORCHESTRATORS
# ==========================================
def run_suite_A(filepath):
    print(f"Generating charts for Test Suite A...\n")
    df = load_and_prep_data(filepath)
    setup_directories()
    
    plot_algo_performance_vs_k(df)
    plot_heap_complexity(df)
    plot_update_complexity(df)
    plot_mean_case_analysis(df)
    print("\nSuite A plotting complete.")

def run_suite_B(filepath):
    print(f"Generating charts for Test Suite B...\n")
    df = load_and_prep_data(filepath)
    setup_directories()
    verify_operations(df, "Suite B")
    plot_normalized_time_complexity(df, varying_var='m', fixed_var='n', fixed_val='2^20', suite_name='B')
    print("\nSuite B plotting complete.")

def run_suite_C(filepath):
    print(f"Generating charts for Test Suite C...\n")
    df = load_and_prep_data(filepath)
    setup_directories()
    verify_operations(df, "Suite C")
    plot_normalized_time_complexity(df, varying_var='n', fixed_var='m', fixed_val='2^20', suite_name='C')
    print("\nSuite C plotting complete.")

def run_suite_D(file_perf, file_report):
    """Orchestrator for Test Suite D (DIMACS real-world graphs)."""
    # Simply calls the summary function which handles its own loading and plotting
    plot_suite_D_summary(file_perf, file_report)

if __name__ == "__main__":
    custom_epilog = """
-------------------------------------------------------------------------------
EXAMPLES:
  Plot Suite A: python generate_plots.py --suite A
  Plot Suite B: python generate_plots.py --suite B --file reports/report_time_fixed_n.csv
  Plot Suite C: python generate_plots.py --suite C --file reports/report_time_fixed_m.csv
  Plot Suite D: python generate_plots.py --suite D --fileD1 performance_metrics_D.csv --fileD2 report_dimacs_real.csv
  Regression:   python generate_plots.py --suite R --fileA reports/report_operations.csv --fileB reports/report_time_fixed_n.csv
-------------------------------------------------------------------------------
"""

    parser = argparse.ArgumentParser(
        description="Modular Python Plotter for Dijkstra Benchmark Data.", 
        formatter_class=argparse.RawTextHelpFormatter, 
        epilog=custom_epilog
    )
    
    # ADDED 'D' to choices
    parser.add_argument('--suite', type=str, choices=['A', 'B', 'C', 'D', 'R'], required=True, help="Select which suite to process (R for Regression).")
    
    # Existing file arguments
    parser.add_argument('--file', type=str, default='reports/report_operations.csv', help="Target CSV file for Suites A, B, and C.")
    parser.add_argument('--fileA', type=str, default='reports/report_operations.csv', help="First file for regression.")
    parser.add_argument('--fileB', type=str, default='reports/report_time_fixed_n.csv', help="Second file for regression.")
    
    # NEW file arguments specifically for Suite D
    parser.add_argument('--fileD1', type=str, default='performance_metrics_D.csv', help="Performance metrics for Suite D.")
    parser.add_argument('--fileD2', type=str, default='reports/report_dimacs_real.csv', help="Real DIMACS report for Suite D.")
    
    args = parser.parse_args()
    
    # Routing execution based on the selected suite
    if args.suite == 'A':
        run_suite_A(args.file)
    elif args.suite == 'B':
        run_suite_B(args.file)
    elif args.suite == 'C':
        run_suite_C(args.file)
    elif args.suite == 'D':
        run_suite_D(args.fileD1, args.fileD2)
    elif args.suite == 'R':
        perform_multiple_linear_regression(args.fileA, args.fileB)