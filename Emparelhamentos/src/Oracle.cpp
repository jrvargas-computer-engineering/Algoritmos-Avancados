#include "Oracle.h"
#include <algorithm>

OracleResult Oracle::evaluate(const BipartiteGraph& graph) {
    int n = graph.get_n();
    std::vector<bool> matched_T(n, false);
    
    OracleResult best_result = {0, -1000000000}; // Bound configured to capture absolute negative domains
    enumerate_subsets(graph, 0, 0, 0, matched_T, best_result);
    return best_result;
}

void Oracle::enumerate_subsets(const BipartiteGraph& graph, int u, int current_size, int current_weight, 
                               std::vector<bool>& matched_T, OracleResult& best_result) {
    int n = graph.get_n();
    
    // Base case: all vertices in S evaluated
    if (u == n) {
        if (current_size > best_result.max_size) {
            best_result.max_size = current_size;
            best_result.max_weight = current_weight;
        } else if (current_size == best_result.max_size) {
            if (current_weight > best_result.max_weight) {
                best_result.max_weight = current_weight;
            }
        }
        return;
    }

    // Branch 1: Vertex u remains isolated (unmatched)
    enumerate_subsets(graph, u + 1, current_size, current_weight, matched_T, best_result);

    // Branch 2: Enumerate all valid mapping configurations for vertex u
    for (int v = 0; v < n; ++v) {
        if (graph.has_edge(u, v) && !matched_T[v]) {
            matched_T[v] = true;
            int edge_weight = graph.get_weight(u, v);
            
            enumerate_subsets(graph, u + 1, current_size + 1, current_weight + edge_weight, matched_T, best_result);
            
            matched_T[v] = false; // Retract state to evaluate parallel permutation branches
        }
    }
}