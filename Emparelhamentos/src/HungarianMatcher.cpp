#include "HungarianMatcher.h"
#include "BenchmarkRegistry.h"

HungarianMatcher::HungarianMatcher(std::shared_ptr<IPathfindingStrategy> path_strategy) 
    : strategy(std::move(path_strategy)) {}

int HungarianMatcher::match(const BipartiteGraph& g) {
    int n = g.get_n();
    std::vector<int> match_S(n, -1);
    std::vector<int> match_T(n, -1);
    
    std::vector<int> predecessors(n * 2, -1);
    
    // Corrected: Utilize the decoupled interface constant
    std::vector<int> distances(n * 2, IPathfindingStrategy::INF); 

    #ifdef BENCHMARK_MODE
    strategy->reset_metrics();
    int augmentation_index = 0;
    int current_defect = n;
    #endif

    while (strategy->find_shortest_augmenting_path(g, match_S, match_T, predecessors, distances)) {
        
        int current = -1;
        
        // Corrected: Utilize the decoupled interface constant
        int min_dist = IPathfindingStrategy::INF; 
        
        for (int v = 0; v < n; ++v) {
            if (match_T[v] == -1 && distances[n + v] < min_dist) {
                min_dist = distances[n + v];
                current = n + v;
            }
        }
        
        if (current == -1) break;

        // Augment the path by backtracking through the predecessor matrix
        while (current != -1) {
            int prev = predecessors[current];
            if (prev == -1) break;
            
            // If transitioning from S to T, update the matching
            if (prev < n && current >= n) {
                match_S[prev] = current - n;
                match_T[current - n] = prev;
            }
            current = predecessors[prev];
        }
    
        #ifdef BENCHMARK_MODE
            current_defect--;
            EMIT_METRIC(augmentation_index, "defect_at_iteration", current_defect);
            augmentation_index++;
            #endif
    }

// Compute final maximum weight matching sum AND size
    int total_weight = 0;
    int matching_size = 0;
    for (int u = 0; u < n; ++u) {
        if (match_S[u] != -1) {
            total_weight += g.get_weight(u, match_S[u]);
            matching_size++;
        }
    }

    #ifdef BENCHMARK_MODE
    EMIT_METRIC(-1, "augmentation_count", augmentation_index);
    EMIT_METRIC(-1, "matching_value", total_weight);
    EMIT_METRIC(-1, "matching_size", matching_size); // <-- INJECT THIS LINE

// Extract stateful strategy metrics
    long long total_relaxations = strategy->get_total_edge_relaxations();
    if (total_relaxations > 0) {
        EMIT_METRIC(-1, "total_edge_relaxations", total_relaxations);
    }
    
    int dijkstra_calls = strategy->get_dijkstra_calls();
    if (dijkstra_calls > 0) {
        EMIT_METRIC(-1, "total_dijkstra_calls", dijkstra_calls);
        EMIT_METRIC(-1, "potential_violations_count", strategy->get_potential_violations_count());
        EMIT_METRIC(-1, "potential_update_overhead_ms", strategy->get_potential_update_overhead_ms());
    }
    #endif
    return total_weight;
}