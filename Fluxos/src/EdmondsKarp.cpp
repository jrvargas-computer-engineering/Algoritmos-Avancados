#include "MaxFlowSolvers.hpp"
#include <queue>
#include <algorithm>
#include <chrono>

MetricsReport EdmondsKarp::solve(FlowNetwork& network) {
    MetricsReport report;
    auto start_time = std::chrono::high_resolution_clock::now();

    int n = network.get_num_vertices();
    int s = network.get_source();
    int t = network.get_sink();

    while (true) {
        int vertices_touched = 0;
        int edges_touched = 0;

        // Arrays to reconstruct the path
        std::vector<int> parent_node(n, -1);
        std::vector<int> parent_edge_idx(n, -1);
        std::queue<int> q;

        q.push(s);
        parent_node[s] = s;
        bool reached_sink = false;

        // BFS traversal
        while (!q.empty() && !reached_sink) {
            int u = q.front();
            q.pop();
            vertices_touched++;

            for (size_t i = 0; i < network.adj_list[u].size(); ++i) {
                edges_touched++; // Track every edge we look at
                const Edge& edge = network.adj_list[u][i];
                
                if (parent_node[edge.to] == -1 && network.get_residual_capacity(edge) > 0) {
                    parent_node[edge.to] = u;
                    parent_edge_idx[edge.to] = i;
                    q.push(edge.to);

                    if (edge.to == t) {
                        reached_sink = true;
                        break;
                    }
                }
            }
        }

        if (!reached_sink) break; // Phase ends: No more augmenting paths exist

        // 1. Find the bottleneck capacity along the discovered path
        long long bottleneck = 1e18; // Start with a very large number
        int curr = t;
        while (curr != s) {
            int p = parent_node[curr];
            int idx = parent_edge_idx[curr];
            bottleneck = std::min(bottleneck, network.get_residual_capacity(network.adj_list[p][idx]));
            curr = p;
        }

        // 2. Augment the flow along the path
        curr = t;
        while (curr != s) {
            int p = parent_node[curr];
            int idx = parent_edge_idx[curr];
            network.augment_flow(p, idx, bottleneck);
            curr = p;
        }

        // 3. Record metrics for this phase
        report.max_flow_value += bottleneck;
        report.total_phases++;
        report.vertices_touched_per_phase.push_back(vertices_touched);
        report.edges_touched_per_phase.push_back(edges_touched);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    report.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return report;
}