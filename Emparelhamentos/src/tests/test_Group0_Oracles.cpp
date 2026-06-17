#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <memory>
#include "BipartiteGraph.h"
#include "GraphGenerator.h"
#include "Oracle.h"
#include "SimpleAugmentingMatcher.h"
#include "HopcroftKarpMatcher.h"
#include "HungarianMatcher.h"
#include "BellmanFordStrategy.h"
#include "JohnsonDijkstraStrategy.h"

void execute_u0_unweighted_gate() {
    std::cout << "[RUN] U0: Small-Instance Oracle Check (Unweighted)\n";
    std::vector<int> n_sizes = {2, 3, 4, 5, 6};
    std::vector<double> alphas = {1.0, 1.5, 2.0};
    int repetitions = 50;

    SimpleAugmentingMatcher simple_matcher;
    HopcroftKarpMatcher hk_matcher;

    int total_evaluations = 0;
    for (int n : n_sizes) {
        for (double alpha : alphas) {
            for (int r = 0; r < repetitions; ++r) {
                // Ensure distinct topologies per repetition
                unsigned int random_seed = (n * 1000) + (static_cast<int>(alpha * 100)) + r;
                BipartiteGraph graph = GraphGenerator::generate(n, alpha, random_seed);

                OracleResult truth = Oracle::evaluate(graph);
                int simple_size = simple_matcher.match(graph);
                int hk_size = hk_matcher.match(graph);

                assert(simple_size == truth.max_size && "SimpleAugmentingMatcher violated U0 exact sizing limit");
                assert(hk_size == truth.max_size && "HopcroftKarpMatcher violated U0 exact sizing limit");
                total_evaluations++;
            }
        }
    }
    std::cout << "      -> STATUS: PASS (" << total_evaluations << " unique matrices verified)\n";
}

void execute_w0_weighted_gate() {
    std::cout << "[RUN] W0: Small-Instance Oracle Check (Weighted)\n";
    std::vector<int> n_sizes = {2, 3, 4};
    double alpha = 1.5;
    int repetitions = 100;

    auto bf_strategy = std::make_shared<BellmanFordStrategy>();
    auto jd_strategy = std::make_shared<JohnsonDijkstraStrategy>();
    HungarianMatcher hungarian_bf(bf_strategy);
    HungarianMatcher hungarian_jd(jd_strategy);

    int total_evaluations = 0;
    for (int n : n_sizes) {
        for (int r = 0; r < repetitions; ++r) {
            unsigned int random_seed = (n * 1000) + r;
            BipartiteGraph graph = GraphGenerator::generate(n, alpha, random_seed);

            // W0 specification forces negative constraints: weights uniform in [-n^2, n^2]
            int limit = n * n;
            for (int u = 0; u < n; ++u) {
                for (int v = 0; v < n; ++v) {
                    if (graph.has_edge(u, v)) {
                        // Deterministic uniform mapping simulation
                        int pseudo_random_weight = -limit + ((u + v * r + random_seed) % (2 * limit + 1));
                        graph.set_edge(u, v, pseudo_random_weight);
                    }
                }
            }

            OracleResult truth = Oracle::evaluate(graph);
            
            // The algorithm must be evaluated based on cardinality first, weight second
            int bf_weight = hungarian_bf.match(graph);
            int jd_weight = hungarian_jd.match(graph);

            // Note: The strategy tracks optimal weight exclusively at max cardinality.
            assert(bf_weight == truth.max_weight && "BellmanFordStrategy violated W0 exact weight limit");
            assert(jd_weight == truth.max_weight && "JohnsonDijkstraStrategy violated W0 exact weight limit");
            total_evaluations++;
        }
    }
    std::cout << "      -> STATUS: PASS (" << total_evaluations << " unique matrices verified)\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "EXECUTING PHASE 3: ORACLE CORRECTNESS GATES\n";
    std::cout << "==================================================\n";
    
    execute_u0_unweighted_gate();
    execute_w0_weighted_gate();
    
    std::cout << "==================================================\n";
    std::cout << "ALL PHASE 3 ORACLE CONSTRAINTS SATISFIED\n";
    std::cout << "==================================================\n";
    return 0;
}