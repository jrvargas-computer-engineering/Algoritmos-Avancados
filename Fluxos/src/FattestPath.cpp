#include "MaxFlowSolvers.hpp"
#include "IndexedMaxHeap.hpp"
#include <algorithm>
#include <chrono>

MetricsReport FattestPath::solve(FlowNetwork& network) {
    MetricsReport report;
    auto start_time = std::chrono::high_resolution_clock::now();

    int n = network.get_num_vertices();
    int s = network.get_source();
    int t = network.get_sink();

    while (true) {
        int vertices_touched = 0;
        int edges_touched = 0;

        std::vector<long long> max_cap(n, 0);
        std::vector<int> parent_node(n, -1);
        std::vector<int> parent_edge_idx(n, -1);
        IndexedMaxHeap pq(n);

        max_cap[s] = 1e18; // Infinity
        pq.insert(s, max_cap[s]);

        while (!pq.is_empty()) {
            int u = pq.extract_max();
            vertices_touched++;

            if (u == t) break; // Found the fattest path to the sink

            for (size_t i = 0; i < network.adj_list[u].size(); ++i) {
                edges_touched++;
                const Edge& edge = network.adj_list[u][i];
                long long residual = network.get_residual_capacity(edge);
                
                if (residual > 0) {
                    // Bottleneck is the minimum of the path to 'u' and the edge capacity
                    long long bottleneck = std::min(max_cap[u], residual);
                    
                    // Relaxation: Maximize the bottleneck
                    if (bottleneck > max_cap[edge.to]) {
                        max_cap[edge.to] = bottleneck;
                        parent_node[edge.to] = u;
                        parent_edge_idx[edge.to] = i;

                        if (pq.contains(edge.to)) {
                            pq.update_key(edge.to, bottleneck);
                        } else {
                            pq.insert(edge.to, bottleneck);
                        }
                    }
                }
            }
        }

        if (max_cap[t] == 0) break; // No more augmenting paths exist

        long long flow_to_push = max_cap[t];

        // Augment flow along the path
        int curr = t;
        while (curr != s) {
            int p = parent_node[curr];
            int idx = parent_edge_idx[curr];
            network.augment_flow(p, idx, flow_to_push);
            curr = p;
        }

        // Record metrics for this phase
        report.max_flow_value += flow_to_push;
        report.total_phases++;
        report.vertices_touched_per_phase.push_back(vertices_touched);
        report.edges_touched_per_phase.push_back(edges_touched);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    report.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return report;
}