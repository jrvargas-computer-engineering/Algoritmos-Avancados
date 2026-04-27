import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os
import argparse

# --- 1. Command Line Interface Setup ---
parser = argparse.ArgumentParser(description="Analyze Maximum Flow Complexity Data.")
parser.add_argument("--density", action="store_true", help="Analyze and plot ONLY the Density Impact experiment.")
parser.add_argument("--scaling", action="store_true", help="Analyze and plot ONLY the Scaling Impact experiment.")
parser.add_argument("--family", type=str, help="Filter by a specific graph family (e.g., BasicLine or DoubleExpLine).")
parser.add_argument("--all", action="store_true", help="Run full analysis on all data (Default).")

args = parser.parse_args()

# Default to --all if no specific flags are provided
if not args.density and not args.scaling and not args.family and not args.all:
    print("No specific flags provided. Defaulting to full analysis (--all).")
    args.all = True

# --- Configuration ---
CSV_FILE = "experiments/reports/final_complexity_report_20260426_231558.csv"
OUTPUT_MD = "complexity_analysis_report.md"
PLOT_DIR = "plots"

if not os.path.exists(PLOT_DIR):
    os.makedirs(PLOT_DIR)

sns.set_theme(style="whitegrid")

# --- 2. Load and Prepare Data ---
if not os.path.exists(CSV_FILE):
    print(f"Error: {CSV_FILE} not found. Please run your Perl experiments script first.")
    exit(1)

print(f"Loading data from {CSV_FILE}...")
df = pd.read_csv(CSV_FILE)

# Apply CLI Filters to the DataFrame
if args.density and not args.all:
    df = df[df['ExpType'] == 'Density']
    print(" -> Filter applied: ONLY Density experiments.")
if args.scaling and not args.all:
    df = df[df['ExpType'] == 'Scaling']
    print(" -> Filter applied: ONLY Scaling experiments.")
if args.family:
    df = df[df['Family'] == args.family]
    print(f" -> Filter applied: ONLY Family '{args.family}'.")

if df.empty:
    print("Error: The applied filters resulted in an empty dataset. Check your CSV or flags.")
    exit(1)

# Helper function to define f(n,m)
def get_f_nm(row):
    algo = row['Algorithm']
    n, m = row['n'], row['m']
    if algo == 'FP':
        return m * np.log2(n) if n > 1 else m  
    else:
        return m  

df['f_nm'] = df.apply(get_f_nm, axis=1)

# Calculate Residual Complexity Ratios
df['Theoretical_Ratio'] = df['Time_ms'] / (df['F_bar'] * df['f_nm'])
df['Empirical_Ops'] = df['F'] * (df['s_bar'] * df['n'] + df['t_bar'] * df['m'])
df['Empirical_Ratio'] = df['Time_ms'] / df['Empirical_Ops'].replace(0, np.nan)

# --- 3. Graph Generation ---
print("Generating graphs...")

def plot_metric(data, x_col, y_col, hue_col, title, filename, ylabel):
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=data, x=x_col, y=y_col, hue=hue_col, marker='o', linewidth=2)
    plt.title(title, fontsize=14)
    plt.ylabel(ylabel, fontsize=12)
    plt.xlabel('Edges (m)' if x_col == 'm' else 'Vertices (n)', fontsize=12)
    plt.legend(title="Algorithm")
    plt.tight_layout()
    plt.savefig(os.path.join(PLOT_DIR, filename))
    plt.close()

families = df['Family'].unique()
exp_types = df['ExpType'].unique()

for family in families:
    for exp in exp_types:
        subset = df[(df['Family'] == family) & (df['ExpType'] == exp)]
        if subset.empty:
            continue
            
        x_axis = 'm' if exp == 'Density' else 'n'
        
        plot_metric(subset, x_axis, 'Time_ms', 'Algorithm', 
                    f'Execution Time vs {exp} ({family})', f'time_{exp.lower()}_{family}.png', 'Time (ms)')
        plot_metric(subset, x_axis, 'r', 'Algorithm', 
                    f'Phase Inadequacy (r) vs {exp} ({family})', f'r_{exp.lower()}_{family}.png', 'r = F / F_bar')
        plot_metric(subset, x_axis, 't_bar', 'Algorithm', 
                    f'Edge Defect (t_bar) vs {exp} ({family})', f't_bar_{exp.lower()}_{family}.png', 'Fraction of Edges Touched')

# --- 4. Markdown Report Generation ---
print("Generating Markdown report...")

with open(OUTPUT_MD, 'w') as f:
    f.write("# Maximum Flow Algorithms: Complexity Analysis Report\n\n")
    
    # Only include Phase Inadequacy if 'r' exists in the filtered data
    f.write("## 1. Inadequacy of Phases ($r$)\n")
    phase_table = df.groupby(['Algorithm', 'ExpType'])['r'].mean().unstack().round(6)
    f.write(phase_table.to_markdown())
    f.write("\n\n")
    
    f.write("## 2. Inadequacy of Operations ($\\bar{s}$ and $\\bar{t}$)\n")
    ops_table = df.groupby(['Algorithm'])[['s_bar', 't_bar']].mean().round(4)
    f.write(ops_table.to_markdown())
    f.write("\n\n")
    
    f.write("## 3. Residual Complexity Analysis\n")
    resid_table = df.groupby(['Algorithm'])[['Theoretical_Ratio', 'Empirical_Ratio']].mean().map(lambda x: f"{x:.2e}")
    f.write(resid_table.to_markdown())
    f.write("\n\n")

    f.write("## 4. Visualizations\n\n")
    for family in families:
        f.write(f"### Graph Family: {family}\n")
        
        if 'Density' in exp_types:
            f.write(f"#### Experiment A: Density Impact (Fixed $n$, increasing $m$)\n")
            f.write(f"![Time vs Density]({PLOT_DIR}/time_density_{family}.png)\n")
            f.write(f"![r vs Density]({PLOT_DIR}/r_density_{family}.png)\n")
            f.write(f"![t_bar vs Density]({PLOT_DIR}/t_bar_density_{family}.png)\n\n")

        if 'Scaling' in exp_types:
            f.write(f"#### Experiment B: Scaling Impact (Fixed density, increasing $n$)\n")
            f.write(f"![Time vs Scaling]({PLOT_DIR}/time_scaling_{family}.png)\n")
            f.write(f"![r vs Scaling]({PLOT_DIR}/r_scaling_{family}.png)\n")
            f.write(f"![t_bar vs Scaling]({PLOT_DIR}/t_bar_scaling_{family}.png)\n\n")

print(f"Done! Open '{OUTPUT_MD}' to view your formatted report.")