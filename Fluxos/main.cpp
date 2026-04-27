#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>

#include "DimacsParser.hpp"
#include "FlowNetwork.hpp"
#include "MaxFlowSolvers.hpp"
#include "MaxFlowSolver.hpp"

// Helper function to print a formatted CSV row
void print_csv_row(const std::string& algo_name, int n, int m, 
                   const MetricsReport& report, double f_bar, int divisor_s) {
    
    double r = (f_bar > 0) ? (static_cast<double>(report.total_phases) / f_bar) : 0;
    
    // Uses your class's built-in calculators!
    double s_bar = report.calculate_s_bar(n, divisor_s);
    double t_bar = report.calculate_t_bar(m, report.total_phases);

    std::cout << algo_name << ","
              << n << ","
              << m << ","
              << report.max_flow_value << ","
              << report.total_phases << ","
              << std::fixed << std::setprecision(2) << f_bar << ","
              << std::fixed << std::setprecision(4) << r << ","
              << std::fixed << std::setprecision(4) << s_bar << ","
              << std::fixed << std::setprecision(4) << t_bar << ","
              << std::fixed << std::setprecision(3) << report.execution_time_ms 
              << "\n";
}

int main(int argc, char* argv[]) {
    // Check if we are running the automated experiments via Perl
    bool isExperiment = (argc > 1 && std::string(argv[1]) == "--benchmark");

    try {
        // 1. Read the network from standard input (DimacsParser returns the object directly)
        FlowNetwork network = DimacsParser::parse_from_stream(std::cin);
        
        int n = network.get_num_vertices();
        int m = network.get_num_edges();

        // =================================================================
        // GRADING MODE: Just print the max flow integer and exit immediately
        // =================================================================
        if (!isExperiment) {
            Dinitz dinitz; 
            MetricsReport report = dinitz.solve(network);
            std::cout << report.max_flow_value << std::endl; 
            return 0; 
        }

        // =================================================================
        // EXPERIMENT MODE: Generate the CSV data for the report
        // =================================================================

        // Print CSV Header
        std::cout << "Algorithm,n,m,Max_Flow,F,F_bar,r,s_bar,t_bar,Time_ms\n";

        // --- 1. Edmonds-Karp (EK) ---
        EdmondsKarp ek;
        MetricsReport ek_report = ek.solve(network);
        long long C = ek_report.max_flow_value; // We will use this Max Flow as C for the other bounds
        network.reset_flow(); // Clean the graph for the next algorithm

        double ek_f_bar = static_cast<double>(n) * m / 2.0;
        print_csv_row("EK", n, m, ek_report, ek_f_bar, ek_report.total_phases);

        // --- 2. Ford-Fulkerson DFS (FF) ---
        FordFulkersonDFS ff;
        MetricsReport ff_report = ff.solve(network);
        network.reset_flow();
        
        double ff_f_bar = static_cast<double>(C); // C (Max Flow)
        print_csv_row("FF", n, m, ff_report, ff_f_bar, ff_report.total_phases);

        // --- 3. Randomized Ford-Fulkerson (RDFS) ---
        FordFulkersonRandDFS rdfs;
        MetricsReport rdfs_report = rdfs.solve(network);
        network.reset_flow();
        
        print_csv_row("RDFS", n, m, rdfs_report, ff_f_bar, rdfs_report.total_phases);

        // --- 4. Fattest Path (FP) ---
        // Theoretical limit is m * log(C)
        double log_C = (C > 1) ? std::log2(C) : 1;
        double fp_f_bar = m * log_C;
        
        FattestPath fp;
        MetricsReport fp_report = fp.solve(network);
        network.reset_flow();
        
        print_csv_row("FP", n, m, fp_report, fp_f_bar, fp_report.total_phases);

        // --- 5. Capacity Scaling (EC) ---
        CapacityScaling ec;
        MetricsReport ec_report = ec.solve(network);
        network.reset_flow();
        
        print_csv_row("EC", n, m, ec_report, fp_f_bar, ec_report.total_phases);

        // --- 6. Dinitz (Di) ---
        Dinitz di;
        MetricsReport di_report = di.solve(network);
        network.reset_flow();
        
        double di_f_bar = static_cast<double>(n); // Pessimistic limit is n phases
        // Crucial: divisor for s_bar in Dinitz is total_iterations (I), NOT total_phases (F)
        print_csv_row("Di", n, m, di_report, di_f_bar, di_report.total_iterations);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}