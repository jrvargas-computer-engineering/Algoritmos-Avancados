#include "JohnsonDijkstraStrategy.h"
#include "BenchmarkRegistry.h"
#include <queue>
#include <algorithm>
#include <stdexcept>
#include <chrono>

void JohnsonDijkstraStrategy::initialize_potentials(const BipartiteGraph& g) {
    int n = g.get_n();
    potentials.assign(2 * n, 0);

    int max_weight = 0;
    bool has_edges = false;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (g.has_edge(i, j)) {
                max_weight = std::max(max_weight, g.get_weight(i, j));
                has_edges = true;
            }
        }
    }

    int W = has_edges ? max_weight : 0;
    for (int v = 0; v < n; ++v) {
        potentials[n + v] = -W;
    }
}

bool JohnsonDijkstraStrategy::find_shortest_augmenting_path(
    const BipartiteGraph& g, 
    const std::vector<int>& match_S, 
    const std::vector<int>& match_T, 
    std::vector<int>& predecessors,
    std::vector<int>& distances) 
{


    #ifdef BENCHMARK_MODE
    int current_call_relaxations = 0;
    dijkstra_call_count++;
    #endif

    int n = g.get_n();
    
    // Automatically reset initialization state if a new matching pass is detected
    bool is_first_iteration = true;
    for (int u = 0; u < n; ++u) {
        if (match_S[u] != -1) {
            is_first_iteration = false;
            break;
        }
    }
    if (is_first_iteration) {
        initialized = false;
    }

    if (!initialized) {
        initialize_potentials(g);
        initialized = true;
    }

    distances.assign(n * 2, INF);
    predecessors.assign(n * 2, -1);

    // Min-priority queue tracking pairs of (distance, vertex_id)
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;

    for (int u = 0; u < n; ++u) {
        if (match_S[u] == -1) {
            distances[u] = 0;
            pq.push({0, u});
        }
    }

    while (!pq.empty()) {
        auto [d, curr] = pq.top();
        pq.pop();

        if (d > distances[curr]) continue;

        if (curr < n) {
            int u = curr;
            for (int v = 0; v < n; ++v) {
                if (g.has_edge(u, v) && match_S[u] != v) {
                    int transformed_cost = get_transformed_weight(g, u, v, true);
                    
                    #ifdef BENCHMARK_MODE
                    current_call_relaxations++;
                    if (transformed_cost < 0) {
                        potential_violations++;
                    }
                    #endif
                    
                    if (distances[u] + transformed_cost < distances[n + v]) {
                        distances[n + v] = distances[u] + transformed_cost;
                        predecessors[n + v] = u;
                        pq.push({distances[n + v], n + v});
                    }
                }
            }
        } else {
            int v = curr - n;
            int u = match_T[v];
            if (u != -1 && g.has_edge(u, v)) {
                int transformed_cost = get_transformed_weight(g, u, v, false);
                
                
                #ifdef BENCHMARK_MODE
                current_call_relaxations++;
                if (transformed_cost < 0) {
                    potential_violations++;
                }
                #endif
                
                if (distances[n + v] + transformed_cost < distances[u]) {
                    distances[u] = distances[n + v] + transformed_cost;
                    predecessors[u] = n + v;
                    pq.push({distances[u], u});
                }
            }
        }
    }

    int delta = INF;
    int best_target_v = -1;
    for (int v = 0; v < n; ++v) {
        if (match_T[v] == -1 && distances[n + v] < delta) {
            delta = distances[n + v];
            best_target_v = n + v;
        }
    }

    if (best_target_v == -1) {
        return false;
    }

    #ifdef BENCHMARK_MODE
    // Spec requires per-call isolation of edge relaxations
    int current_augmentation = dijkstra_call_count - 1;
    EMIT_METRIC(current_augmentation, "edge_relaxations", current_call_relaxations);
    total_relaxations += current_call_relaxations;
    
    auto overhead_start = std::chrono::high_resolution_clock::now();
    #endif


    // Update potentials uniformly based on calculated shortest transformed distances
    for (int i = 0; i < n * 2; ++i) {
        if (distances[i] <= delta) {
            potentials[i] += distances[i];
        } else {
            potentials[i] += delta;
        }
    }

    #ifdef BENCHMARK_MODE
    auto overhead_end = std::chrono::high_resolution_clock::now();
    accumulated_overhead_ms += std::chrono::duration<double, std::milli>(overhead_end - overhead_start).count();
    #endif

    return true;
}

const std::vector<int>& JohnsonDijkstraStrategy::get_potentials() const {
    return potentials;
}

int JohnsonDijkstraStrategy::get_transformed_weight(const BipartiteGraph& g, int u, int v, bool is_forward) const {
    int n = g.get_n();
    if (is_forward) {
        int orig_cost = -g.get_weight(u, v);
        return orig_cost - (potentials[n + v] - potentials[u]);
    } else {
        int orig_cost = g.get_weight(u, v);
        return orig_cost - (potentials[u] - potentials[n + v]);
    }
}

#ifdef BENCHMARK_MODE
void JohnsonDijkstraStrategy::reset_metrics() {
    dijkstra_call_count = 0;
    potential_violations = 0;
    total_relaxations = 0;
    accumulated_overhead_ms = 0.0;
}
#endif