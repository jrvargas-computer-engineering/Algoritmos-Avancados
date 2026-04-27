#include "MaxFlowSolvers.hpp"
#include <queue>
#include <algorithm>
#include <chrono>

class DinitzImpl {
    FlowNetwork& network;
    std::vector<int> level;
    std::vector<int> ptr; // "Dead-end" pruning array
    int s, t, n;
    int phase_edges_touched;

public:
    DinitzImpl(FlowNetwork& net) : network(net) {
        n = network.get_num_vertices();
        s = network.get_source();
        t = network.get_sink();
    }

    bool bfs(int& vertices_touched) {
        level.assign(n, -1);
        std::queue<int> q;
        level[s] = 0;
        q.push(s);

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            vertices_touched++;

            for (const auto& edge : network.adj_list[u]) {
                phase_edges_touched++;
                if (level[edge.to] == -1 && network.get_residual_capacity(edge) > 0) {
                    level[edge.to] = level[u] + 1;
                    q.push(edge.to);
                }
            }
        }
        return level[t] != -1;
    }

    long long dfs(int u, long long pushed, int& vertices_touched) {
        if (pushed == 0) return 0;
        if (u == t) return pushed;
        
        vertices_touched++;

        // ptr[u] prevents revisiting dead-ends in the level graph
        for (int& cid = ptr[u]; cid < network.adj_list[u].size(); ++cid) {
            phase_edges_touched++;
            Edge& edge = network.adj_list[u][cid];
            long long res = network.get_residual_capacity(edge);

            // Only traverse edges progressing strictly to the next level
            if (level[u] + 1 != level[edge.to] || res == 0) continue;

            long long push = dfs(edge.to, std::min(pushed, res), vertices_touched);
            if (push == 0) continue;

            network.augment_flow(u, cid, push);
            return push;
        }
        return 0;
    }

    MetricsReport solve() {
        MetricsReport report;
        auto start_time = std::chrono::high_resolution_clock::now();

        while (true) {
            int bfs_vertices_touched = 0;
            phase_edges_touched = 0; // Reset edge counter for the new phase

            // 1. Build level graph
            if (!bfs(bfs_vertices_touched)) break; // Sink unreachable, phase ends
            
            report.total_phases++;
            ptr.assign(n, 0);

            long long phase_flow = 0;
            
            // 2. Find blocking flows (Iterations)
            while (true) {
                int dfs_vertices_touched = 0;
                long long pushed = dfs(s, 1e18, dfs_vertices_touched);
                
                report.total_iterations++;
                report.vertices_touched_per_phase.push_back(dfs_vertices_touched);

                if (pushed == 0) break; // Blocking flow achieved
                phase_flow += pushed;
            }
            
            report.max_flow_value += phase_flow;
            report.edges_touched_per_phase.push_back(phase_edges_touched);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        report.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        return report;
    }
};

MetricsReport Dinitz::solve(FlowNetwork& network) {
    return DinitzImpl(network).solve();
}