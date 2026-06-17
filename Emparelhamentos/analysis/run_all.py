# analysis/run_all.py
import argparse
import os
import sys

# Import all plotting modules
from analysis.plots import u0, w0, u1, u2, u3, u4, w1, w2, w3, w4, c1, c2

def main():
    parser = argparse.ArgumentParser(description="Bipartite Matching - Master Analysis Orchestrator")
    parser.add_argument("--data-dir", default="data/combined/", help="Directory containing aggregated CSVs")
    parser.add_argument("--out-dir", default="analysis_output/", help="Target directory for report tables and figures")
    args = parser.parse_args()

    # Ensure output directories exist
    os.makedirs(args.out_dir, exist_ok=True)
    os.makedirs(os.path.join(args.out_dir, "figures"), exist_ok=True)

    print("==================================================")
    print("STARTING BIPARTITE MASTER ANALYSIS PIPELINE")
    print("==================================================")

    # 1. GROUP 0: ORACLE CORRECTNESS GATES
    # These MUST pass. If they fail, sys.exit(1) is triggered internally.
    u0.run(args.data_dir, args.out_dir)
    w0.run(args.data_dir, args.out_dir)

    # 2. INNER-LOOP CORRECTNESS GATES
    # These modules verify parity, potential limits, and defect traces.
    u1.run(args.data_dir, args.out_dir) # Triggers Output 2 (matching_size == n)
    w1.run(args.data_dir, args.out_dir) # Triggers Output 9 (parity)
    w2.run(args.data_dir, args.out_dir) # Triggers Output 12 (potential violations)
    w4.run(args.data_dir, args.out_dir) # Triggers Outputs 17/17b (negative stress tests)

    # 3. EMPIRICAL SCALABILITY AND TRACES
    u2.run(args.data_dir, args.out_dir)
    u3.run(args.data_dir, args.out_dir)
    u4.run(args.data_dir, args.out_dir)
    w3.run(args.data_dir, args.out_dir)

    # 4. MASTER REPORT TABLES
    c1.run(args.data_dir, args.out_dir)
    c2.run(args.data_dir, args.out_dir)

    print("==================================================")
    print("ANALYSIS PIPELINE COMPLETED SUCCESSFULLY")
    print(f"All artifacts written to: {args.out_dir}")
    print("==================================================")

if __name__ == "__main__":
    main()