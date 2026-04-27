#include "MaxFlowSolvers.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <numeric>

// Recursive DFS helper
long long FordFulkersonRandDFS::dfs(FlowNetwork& network, int u, int t, long long current_flow, 
                                std::vector<bool>& visited, int& v_touched, int& e_touched) {
    if (u == t) return current_flow;
    
    visited[u] = true;
    v_touched++;

    std::vector<size_t> indices(network.adj_list[u].size());
    std:iota(indices.begin(), indices.end(), 0);

    static std::random_device rd; 
    static std::mt19937 g(rd());

    std::shuffle(indices.begin(), indices.end(), g); 

    for (size_t idx = 0; idx < network.adj_list[u].size(); ++idx) {
        size_t i = indices[idx]; 
        
        e_touched++; // Track every edge evaluated
        Edge& edge = network.adj_list[u][i];
        long long residual = network.get_residual_capacity(edge);
        
        if (!visited[edge.to] && residual > 0) {
            // Recurse deeper, carrying the bottleneck flow downwards
            long long bottleneck = dfs(network, edge.to, t, std::min(current_flow, residual), 
                                       visited, v_touched, e_touched);
            
            // If a path to the sink was found, augment flow on the way back up
            if (bottleneck > 0) {
                network.augment_flow(u, i, bottleneck);
                return bottleneck;
            }
        }
    }
    return 0; // No path found from this node
}

MetricsReport FordFulkersonRandDFS::solve(FlowNetwork& network) {
    MetricsReport report;
    auto start_time = std::chrono::high_resolution_clock::now();

    int n = network.get_num_vertices();
    int s = network.get_source();
    int t = network.get_sink();
    const long long INF = 1e18;

    while (true) {
        std::vector<bool> visited(n, false);
        int vertices_touched = 0;
        int edges_touched = 0;

        // Launch DFS
        long long flow_pushed = dfs(network, s, t, INF, visited, vertices_touched, edges_touched);

        if (flow_pushed == 0) break; // Phase ends: No more augmenting paths exist

        // Record metrics for this phase
        report.max_flow_value += flow_pushed;
        report.total_phases++;
        report.vertices_touched_per_phase.push_back(vertices_touched);
        report.edges_touched_per_phase.push_back(edges_touched);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    report.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return report;
}