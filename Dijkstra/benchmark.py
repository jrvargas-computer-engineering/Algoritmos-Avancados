import os
import sys
import random
import subprocess
import time
import argparse
import heapq
import csv
import math

# Try importing resource for UNIX memory measurement
try:
    import resource
    HAS_RESOURCE = True
except ImportError:
    HAS_RESOURCE = False


# ==========================================
# CORE MODULES
# ==========================================

def python_dijkstra(graph, source, dest):
    """Pure-Python baseline for verification on small graphs."""
    distances = {node: float('inf') for node in graph}
    distances[source] = 0
    pq = [(0, source)]

    while pq:
        current_dist, u = heapq.heappop(pq)
        if current_dist > distances[u]:
            continue
        if dest != -1 and u == dest:
            return str(current_dist)

        for v, weight in graph[u]:
            distance = current_dist + weight
            if distance < distances[v]:
                distances[v] = distance
                heapq.heappush(pq, (distance, v))
    return "inf"

def generate_dimacs_graph(filename, num_vertices, num_edges, build_py_graph=False):
    """Generates the DIMACS file. Avoids memory bloat on massive graphs."""
    graph = {i: [] for i in range(1, num_vertices + 1)} if build_py_graph else None
    
    with open(filename, 'w') as f:
        f.write(f"c Auto-generated distance graph\n")
        f.write(f"p sp {num_vertices} {num_edges}\n")
        
        edges_created = 0
        for i in range(1, num_vertices):
            if edges_created >= num_edges: break
            weight = random.randint(1, 100)
            f.write(f"a {i} {i+1} {weight}\n")
            if build_py_graph: graph[i].append((i+1, weight))
            edges_created += 1
            
        while edges_created < num_edges:
            u, v = random.randint(1, num_vertices), random.randint(1, num_vertices)
            if u != v:
                weight = random.randint(1, 100)
                f.write(f"a {u} {v} {weight}\n")
                if build_py_graph: graph[u].append((v, weight))
                edges_created += 1
                
    return graph

def extract_latest_cpp_metrics():
    """Reads the last line of the C++ performance CSV to extract raw metrics."""
    try:
        with open("performance_metrics.csv", "r") as f:
            lines = f.readlines()
            if len(lines) > 1:
                return lines[-1].strip().split(",")
    except Exception:
        pass
    return None

def get_max_memory(command, stdin_file):
    """Executes a command and measures peak memory (RSS) using the UNIX resource module."""
    if not HAS_RESOURCE:
        return run_cpp(command, stdin_file), 0.0
        
    # Fork a child process to isolate memory tracking
    pid = os.fork()
    if pid == 0:
        # Child process: Run the C++ executable
        with open(stdin_file, 'r') as f:
            subprocess.run(command, stdin=f, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        os._exit(0)
    else:
        # Parent process: Wait for child and read resource usage
        _, status, rusage = os.wait4(pid, 0)
        # ru_maxrss is in kilobytes on Linux
        memory_mb = rusage.ru_maxrss / 1024.0
        return status == 0, memory_mb


# ==========================================
# TEST SUITES
# ==========================================

def run_test_suite_A():
    """Test Suite A: Operation Ratios and Best k Verification."""
    print("Running Test Suite A: Operation Ratios...")
    os.makedirs("reports", exist_ok=True)
    os.makedirs("graphs", exist_ok=True)
    
    k_values = [2, 3, 4, 5, 8, 16]
    
    # The test plan requires collecting statistics over random graphs with 
    # different numbers of vertices (n) and edges (m), with replications.
    n_values = [100000, 250000] 
    replications = 10
    
    with open("reports/report_operations.csv", "w", newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["k", "n", "m", "alpha", "sift_up", "sift_down", "r_up", "r_down", "insert", "deletemin", "update", "i", "d", "u", "noshita_expected", "Algo_Time_ms"])
        
        for n in n_values:
            # Vary graph density: sparse, medium, and dense
            m_values = [n * 2, n * 10, n * 30] 
            
            for m in m_values:
                for rep in range(replications):
                    filename = f"graphs/suiteA_{n}V_{m}E_rep{rep}.gr"
                    generate_dimacs_graph(filename, n, m, build_py_graph=False)
                    
                    for k in k_values:
                        # Full traversal requires starting from a random vertex and not stopping at the destination.
                        source = random.randint(1, n)
                        
                        with open(filename, 'r') as f:
                            subprocess.run(["./dijkstra", str(source), "-1", str(k)], stdin=f, capture_output=True)
                        
                        metrics = extract_latest_cpp_metrics()
                        if metrics:
                            # Extract Algo_Time_ms and operations
                            algo_time = metrics[4]
                            I, D, U = int(metrics[5]), int(metrics[6]), int(metrics[7])
                            sift_up, sift_down = int(metrics[8]), int(metrics[9])
                            
                            # Mathematical derivations based on the test plan
                            alpha = math.log(m, n) if n > 1 else 1
                            log_k_n = math.log(n, k) if k > 1 else 1
                            
                            # Ratios of sifts divided by the worst case (log_k n)
                            total_heap_ops = I + U 
                            avg_sift_up = sift_up / total_heap_ops if total_heap_ops > 0 else 0
                            avg_sift_down = sift_down / total_heap_ops if total_heap_ops > 0 else 0
                            
                            r_up = avg_sift_up / log_k_n if log_k_n > 0 else 0
                            r_down = avg_sift_down / log_k_n if log_k_n > 0 else 0
                            
                            # Ratios for algorithm operations
                            i_ratio = I / n
                            d_ratio = D / n
                            u_ratio = U / m
                            
                            # Expected value of Noshita
                            noshita = (alpha - 1) * n * math.log(n)
                            
                            writer.writerow([k, n, m, f"{alpha:.3f}", sift_up, sift_down, f"{r_up:.3f}", f"{r_down:.3f}", I, D, U, f"{i_ratio:.3f}", f"{d_ratio:.3f}", f"{u_ratio:.3f}", f"{noshita:.3f}", algo_time])
                    os.remove(filename)
                    
    print("Test Suite A Complete. Results saved to reports/report_operations.csv")

def run_test_suite_B(k):
    """Test Suite B: Time Complexity (Varying Edges m)."""
    print("Running Test Suite B: Varying Edges (m)...")
    os.makedirs("reports", exist_ok=True)
    os.makedirs("graphs", exist_ok=True)
    
    n = 2**20
    # i = 41...60. m = sqrt(2)^i
    m_values = [int(math.sqrt(2)**i) for i in range(41, 54)] 
    
    with open("reports/report_time_fixed_n.csv", "w", newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["n", "m", "I", "D", "U", "Algo_Time_ms"])
        
        for m in m_values:
            filename = f"graphs/suiteB_{n}V_{m}E.gr"
            generate_dimacs_graph(filename, n, m, build_py_graph=False)
            
            for _ in range(30):
                source = random.randint(1, n)
                with open(filename, 'r') as f:
                    subprocess.run(["./dijkstra", str(source), "-1", str(k)], stdin=f, capture_output=True)
                
                metrics = extract_latest_cpp_metrics()
                if metrics:
                    algo_time = metrics[4]
                    I, D, U = metrics[5], metrics[6], metrics[7]
                    writer.writerow([n, m, I, D, U, algo_time])
            os.remove(filename)
    print("Test Suite B Complete. Results saved to reports/report_time_fixed_n.csv")


def run_test_suite_C(k):
    """Test Suite C: Time Complexity (Varying Vertices n)."""
    print("Running Test Suite C: Varying Vertices (n)...")
    os.makedirs("reports", exist_ok=True)
    os.makedirs("graphs", exist_ok=True)
    
    m = 2**20
    # i = 11...20. n = 2^i
    n_values = [2**i for i in range(11, 21)] 
    
    with open("reports/report_time_fixed_m.csv", "w", newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["n", "m", "I", "D", "U", "Algo_Time_ms"])
        
        for n in n_values:
            filename = f"graphs/suiteC_{n}V_{m}E.gr"
            generate_dimacs_graph(filename, n, m, build_py_graph=False)
            
            for _ in range(30):
                source = random.randint(1, n)
                with open(filename, 'r') as f:
                    subprocess.run(["./dijkstra", str(source), "-1", str(k)], stdin=f, capture_output=True)
                
                metrics = extract_latest_cpp_metrics()
                if metrics:
                    algo_time = metrics[4]
                    I, D, U = metrics[5], metrics[6], metrics[7]
                    writer.writerow([n, m, I, D, U, algo_time])
            os.remove(filename)
    print("Test Suite C Complete. Results saved to reports/report_time_fixed_m.csv")


def run_test_suite_D(graph_files, k):
    """Test Suite D: Real-World DIMACS Scaling."""
    print("Running Test Suite D: Real-World DIMACS Scaling...")
    os.makedirs("reports", exist_ok=True)
    
    with open("reports/report_dimacs_real.csv", "w", newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Graph", "Run_ID", "Algo_Time_ms", "Memory_MB"])
        
        for graph_file in graph_files:
            if not os.path.exists(graph_file):
                print(f"File {graph_file} not found, skipping.")
                continue
                
            # Determine max vertices from the file to pick a random source
            with open(graph_file, 'r') as f:
                for line in f:
                    if line.startswith('p sp'):
                        n = int(line.split()[2])
                        break
                        
            for run_id in range(1, 31):
                source = random.randint(1, n)
                command = ["./dijkstra", str(source), "-1", str(k)]
                
                # Measure memory via the OS wrapper
                success, peak_ram = get_max_memory(command, graph_file)
                
                if success:
                    metrics = extract_latest_cpp_metrics()
                    algo_time = metrics[4] if metrics else "ERR"
                    writer.writerow([graph_file, run_id, algo_time, f"{peak_ram:.2f}"])
                    print(f"{graph_file} - Run {run_id}/30 | Time: {algo_time}ms | RAM: {peak_ram:.2f}MB")
                    
    print("Test Suite D Complete. Results saved to reports/report_dimacs_real.csv")


# ==========================================
# CLI ORCHESTRATION
# ==========================================
if __name__ == "__main__":
    # Custom help text with detailed examples for each suite
    custom_epilog = """
-------------------------------------------------------------------------------
EXAMPLES AND USAGE:

1. Test Suite A: Operation Ratios and Best 'k' Verification
   Finds the optimal 'k' value by testing k in {2, 3, 4, 5, 8, 16}.
   Outputs to: reports/report_operations.csv
   Command: python3 benchmark.py --suite A

2. Test Suite B: Time Complexity (Varying Edges 'm')
   Fixes n = 2^20 and varies m. Uses the optimal 'k' you specify.
   Outputs to: reports/report_time_fixed_n.csv
   Command: python3 benchmark.py --suite B --k 8

3. Test Suite C: Time Complexity (Varying Vertices 'n')
   Fixes m = 2^20 and varies n. Uses the optimal 'k' you specify.
   Outputs to: reports/report_time_fixed_m.csv
   Command: python3 benchmark.py --suite C --k 8

4. Test Suite D: Real-World DIMACS Scaling
   Tests the algorithm on massive real-world maps and measures memory usage.
   Outputs to: reports/report_dimacs_real.csv
   Command: python3 benchmark.py --suite D --files NY.gr EUA.gr --k 8
-------------------------------------------------------------------------------
"""

    parser = argparse.ArgumentParser(
        description="Modular Benchmark Orchestrator for Dijkstra's Algorithm.\n"
                    "Automates the generation, execution, and data extraction for complex graph scaling tests.",
        formatter_class=argparse.RawTextHelpFormatter, # This preserves the line breaks in our custom text
        epilog=custom_epilog
    )
    
    parser.add_argument('--suite', type=str, choices=['A', 'B', 'C', 'D'], 
                        help="Select which test suite to run (A, B, C, or D).")
    parser.add_argument('--files', nargs='+', 
                        help="Provide paths to real DIMACS files (Required for Suite D).")
    parser.add_argument('--k', type=int, default=4, 
                        help="The k-ary heap parameter to use for Suites B, C, and D. (Default: 4)")
    
    args = parser.parse_args()
    
    # If no arguments are passed at all, print the help menu and exit
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
        
    if not os.path.isfile("./dijkstra"):
        print("Error: Could not find './dijkstra'. Please compile your C++ code first.")
        sys.exit(1)
        
    if args.suite == 'A':
        run_test_suite_A()
    elif args.suite == 'B':
        run_test_suite_B(args.k)
    elif args.suite == 'C':
        run_test_suite_C(args.k)
    elif args.suite == 'D':
        if not args.files:
            print("Error: Suite D requires actual DIMACS files. Use: --files NY.gr EUA.gr")
            sys.exit(1)
        run_test_suite_D(args.files, args.k)
    else:
        print("Please specify a test suite to run using the --suite flag.")
        print("For more info, use: python3 benchmark.py -h")