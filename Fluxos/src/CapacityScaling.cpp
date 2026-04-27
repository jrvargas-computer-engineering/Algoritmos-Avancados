#include "MaxFlowSolvers.hpp"
#include <queue>
#include <algorithm>
#include <chrono>

MetricsReport CapacityScaling::solve(FlowNetwork& network) {
    MetricsReport report;
    auto start_time = std::chrono::high_resolution_clock::now();

    int n = network.get_num_vertices();
    int s = network.get_source();
    int t = network.get_sink();

    // 1. Find the maximum capacity out of the source to initialize Delta
    long long max_cap = 0;
    for (const auto& edge : network.adj_list[s]) {
        max_cap = std::max(max_cap, edge.capacity);
    }

    // Set Delta to the largest power of 2 smaller than or equal to max_cap
    long long delta = 1;
    while (delta <= max_cap) delta *= 2;
    delta /= 2;
    if (delta == 0) delta = 1;

    // 2. Scaling Loop
    while (delta >= 1) {
        bool path_found = true;
        
        // Find augmenting paths using only edges with capacity >= delta
        while (path_found) {
            path_found = false;
            int vertices_touched = 0;
            int edges_touched = 0;

            std::vector<int> parent_node(n, -1);
            std::vector<int> parent_edge_idx(n, -1);
            std::queue<int> q;

            q.push(s);
            parent_node[s] = s;

            while (!q.empty() && !path_found) {
                int u = q.front();
                q.pop();
                vertices_touched++;

                for (size_t i = 0; i < network.adj_list[u].size(); ++i) {
                    edges_touched++;
                    const Edge& edge = network.adj_list[u][i];
                    
                    // The core Scaling restriction:
                    if (parent_node[edge.to] == -1 && network.get_residual_capacity(edge) >= delta) {
                        parent_node[edge.to] = u;
                        parent_edge_idx[edge.to] = i;
                        q.push(edge.to);

                        if (edge.to == t) {
                            path_found = true;
                            break;
                        }
                    }
                }
            }

            if (path_found) {
                long long bottleneck = 1e18;
                int curr = t;
                while (curr != s) {
                    int p = parent_node[curr];
                    int idx = parent_edge_idx[curr];
                    bottleneck = std::min(bottleneck, network.get_residual_capacity(network.adj_list[p][idx]));
                    curr = p;
                }

                curr = t;
                while (curr != s) {
                    int p = parent_node[curr];
                    int idx = parent_edge_idx[curr];
                    network.augment_flow(p, idx, bottleneck);
                    curr = p;
                }

                report.max_flow_value += bottleneck;
                report.total_phases++;
                report.vertices_touched_per_phase.push_back(vertices_touched);
                report.edges_touched_per_phase.push_back(edges_touched);
            }
        }
        // Reduce delta for the next scaling phase
        delta /= 2; 
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    report.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return report;
}