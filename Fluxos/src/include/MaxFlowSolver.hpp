#pragma once
#include <vector>
#include <numeric>
#include "FlowNetwork.hpp"

// Tracks the experimental complexity metrics required by the test plan
struct MetricsReport {
    long long max_flow_value = 0;
    
    // Algorithm Level (Phases)
    int total_phases = 0;       // F
    int total_iterations = 0;   // I (Used specifically for Dinitz)
    
    // Search Level (Operations per phase/iteration)
    std::vector<int> vertices_touched_per_phase; // n'_i
    std::vector<int> edges_touched_per_phase;    // m'_i
    
    // Time
    double execution_time_ms = 0.0;

    // Helper to calculate the average fraction of vertices touched (\bar{s})
    // For FF/EK/FP/EC, 'divisor' will be total_phases (F).
    // For Dinitz, 'divisor' will be total_iterations (I).
    double calculate_s_bar(int total_vertices, int divisor) const {
        if (divisor == 0 || total_vertices == 0) return 0.0;
        
        double sum_s_i = 0.0;
        for (int n_i : vertices_touched_per_phase) {
            sum_s_i += static_cast<double>(n_i) / total_vertices;
        }
        return sum_s_i / divisor;
    }

    // Helper to calculate the average fraction of edges touched (\bar{t})
    double calculate_t_bar(int total_edges, int divisor) const {
        if (divisor == 0 || total_edges == 0) return 0.0;
        
        double sum_t_i = 0.0;
        for (int m_i : edges_touched_per_phase) {
            sum_t_i += static_cast<double>(m_i) / total_edges;
        }
        return sum_t_i / divisor;
    }
};

// Abstract base class for all max-flow algorithms (Strategy Pattern)
class MaxFlowSolver {
public:
    virtual ~MaxFlowSolver() = default;

    // Takes a FlowNetwork by reference and returns the experimental metrics
    virtual MetricsReport solve(FlowNetwork& network) = 0;
};