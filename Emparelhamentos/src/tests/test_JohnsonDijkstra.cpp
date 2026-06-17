#include <iostream>
#include <cassert>
#include <algorithm>
#include <memory>
#include <vector>
#include "BipartiteGraph.h"
#include "HungarianMatcher.h"
#include "JohnsonDijkstraStrategy.h"

// ============================================================================
// NOMINAL INITIALIZATION VERIFICATION
// ============================================================================

void test_initial_potential_bounds() {
    std::cout << "[RUN] test_initial_potential_bounds (p_v Initialization Verification)\n";

    BipartiteGraph graph(3);
    graph.set_edge(0, 0, 10);
    graph.set_edge(1, 1, 25); // Maximum weight W = 25
    graph.set_edge(2, 2, 8);

    std::vector<int> match_S(3, -1);
    std::vector<int> match_T(3, -1);
    std::vector<int> preds(6, -1);
    std::vector<int> dists(6, IPathfindingStrategy::INF);

    JohnsonDijkstraStrategy strategy;
    strategy.find_shortest_augmenting_path(graph, match_S, match_T, preds, dists);

    const auto& potentials = strategy.get_potentials();
    
    std::cout << "      Verifying partition S potentials are initialized to 0...\n";
    for (int u = 0; u < 3; ++u) {
        assert(potentials[u] == 0);
    }

    std::cout << "      Verifying partition T potentials are initialized to -W (-25)...\n";
    for (int v = 0; v < 3; ++v) {
        assert(potentials[3 + v] == -25);
    }
    std::cout << "      -> STATUS: PASS\n";
}

// ============================================================================
// MULTI-ITERATION INVARIANT TRACKING
// ============================================================================

void test_potential_updates_multi_iteration() {
    std::cout << "[RUN] test_potential_updates_multi_iteration (Continuous d'_uv >= 0 Proof)\n";

    BipartiteGraph graph(3);
    graph.set_edge(0, 0, 10); graph.set_edge(0, 1, 5);  graph.set_edge(0, 2, 2);
    graph.set_edge(1, 0, 2);  graph.set_edge(1, 1, 12); graph.set_edge(1, 2, 4);
    graph.set_edge(2, 0, 3);  graph.set_edge(2, 1, 1);  graph.set_edge(2, 2, 15);

    std::vector<int> match_S(3, -1);
    std::vector<int> match_T(3, -1);
    
    JohnsonDijkstraStrategy strategy;

    for (int phase = 0; phase < 3; ++phase) {
        std::vector<int> preds(6, -1);
        std::vector<int> dists(6, IPathfindingStrategy::INF);

        bool path_found = strategy.find_shortest_augmenting_path(graph, match_S, match_T, preds, dists);
        assert(path_found == true);

        std::cout << "      Phase " << phase << ": Interrogating transformed weights post-potential-update...\n";
        for (int u = 0; u < 3; ++u) {
            for (int v = 0; v < 3; ++v) {
                if (graph.has_edge(u, v)) {
                    // Invariants must be checked strictly against arcs present in the active auxiliary topology
                    if (match_S[u] == v) {
                        int reverse_cost = strategy.get_transformed_weight(graph, u, v, false);
                        assert(reverse_cost >= 0);
                    } else {
                        int forward_cost = strategy.get_transformed_weight(graph, u, v, true);
                        assert(forward_cost >= 0);
                    }
                }
            }
        }

        int best_v = -1;
        int min_d = IPathfindingStrategy::INF;
        for (int v = 0; v < 3; ++v) {
            if (match_T[v] == -1 && dists[3 + v] < min_d) {
                min_d = dists[3 + v];
                best_v = 3 + v;
            }
        }
        
        if (best_v != -1) {
            int curr = best_v;
            while (curr != -1) {
                int p = preds[curr];
                if (p == -1) break;
                if (p < 3 && curr >= 3) {
                    match_S[p] = curr - 3;
                    match_T[curr - 3] = p;
                }
                curr = preds[p];
            }
        }
    }
    std::cout << "      -> STATUS: PASS (Potentials maintain non-negativity across all active topological phases)\n";
}

// ============================================================================
// NEGATIVE WEIGHT REGISTRY BOUNDARY TEST
// ============================================================================

void test_potential_all_negative_weights() {
    std::cout << "[RUN] test_potential_all_negative_weights (Negative Domain Guard)\n";

    BipartiteGraph graph(2);
    graph.set_edge(0, 0, -4);
    graph.set_edge(0, 1, -9);
    graph.set_edge(1, 0, -16);
    graph.set_edge(1, 1, -1); 

    std::vector<int> match_S(2, -1);
    std::vector<int> match_T(2, -1);
    std::vector<int> preds(4, -1);
    std::vector<int> dists(4, IPathfindingStrategy::INF);

    JohnsonDijkstraStrategy strategy;
    strategy.find_shortest_augmenting_path(graph, match_S, match_T, preds, dists);

    std::cout << "      Verifying non-negativity under negative weight transformations...\n";
    for (int u = 0; u < 2; ++u) {
        for (int v = 0; v < 2; ++v) {
            if (graph.has_edge(u, v)) {
                int forward_cost = strategy.get_transformed_weight(graph, u, v, true);
                assert(forward_cost >= 0);
            }
        }
    }
    std::cout << "      -> STATUS: PASS\n";
}

// ============================================================================
// END-TO-END INTEGRATION
// ============================================================================

void test_johnson_matcher_integration() {
    std::cout << "[RUN] test_johnson_matcher_integration (End-to-End Correctness)\n";

    BipartiteGraph graph(3);
    graph.set_edge(0, 0, 10); graph.set_edge(0, 1, -5); graph.set_edge(0, 2, 20);
    graph.set_edge(1, 0, 0);  graph.set_edge(1, 1, 15); graph.set_edge(1, 2, -10);
    graph.set_edge(2, 0, -2); graph.set_edge(2, 1, 4);  graph.set_edge(2, 2, 8);

    auto johnson_strategy = std::make_shared<JohnsonDijkstraStrategy>();
    HungarianMatcher matcher(johnson_strategy);

    int max_weight = matcher.match(graph);
    std::cout << "      Calculated Weight: " << max_weight << " | Expected: 33\n";
    assert(max_weight == 33);
    std::cout << "      -> STATUS: PASS\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "EXECUTING STEP 4: HIGH-COVERAGE JOHNSON-DIJKSTRA SUITE\n";
    std::cout << "==================================================\n";
    
    test_initial_potential_bounds();
    test_potential_updates_multi_iteration();
    test_potential_all_negative_weights();
    test_johnson_matcher_integration();
    
    std::cout << "==================================================\n";
    return 0;
}