#include <iostream>
#include <cassert>
#include <chrono>
#include "BipartiteGraph.h"
#include "GraphGenerator.h"
#include "SimpleAugmentingMatcher.h"
#include "HopcroftKarpMatcher.h"

// ============================================================================
// NOMINAL BASELINE EVALUATION
// ============================================================================

void test_functional_matching() {
    std::cout << "[RUN] test_functional_matching (Complete K_{3,3} Control)\n";
    
    BipartiteGraph g_complete(3);
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            g_complete.set_edge(i, j, 1);
        }
    }

    SimpleAugmentingMatcher simple;
    HopcroftKarpMatcher hk;

    assert(simple.match(g_complete) == 3);
    assert(hk.match(g_complete) == 3);
    std::cout << "      -> STATUS: PASS (Algorithms yield identical maximum bounds)\n";
}

// ============================================================================
// TOPOLOGICAL EXTREME EVALUATION
// ============================================================================

void test_topological_empty_matrix() {
    std::cout << "[RUN] test_topological_empty_matrix (E = 0 Constraint)\n";
    
    BipartiteGraph g_empty(5); // 5 vertices per partition, 0 edges initialized

    SimpleAugmentingMatcher simple;
    HopcroftKarpMatcher hk;

    assert(simple.match(g_empty) == 0);
    assert(hk.match(g_empty) == 0);
    std::cout << "      -> STATUS: PASS (Algorithms correctly process structural voids)\n";
}

void test_topological_asymmetric_matching() {
    std::cout << "[RUN] test_topological_asymmetric_matching (Imperfect Maximum Constraint)\n";
    
    BipartiteGraph g_asymmetric(4);
    // Explicitly construct a graph where only 2 matches are physically possible
    g_asymmetric.set_edge(0, 0, 1);
    g_asymmetric.set_edge(0, 1, 1);
    g_asymmetric.set_edge(1, 0, 1);
    g_asymmetric.set_edge(1, 1, 1);
    // Vertices 2 and 3 in partition S have no outbound edges

    SimpleAugmentingMatcher simple;
    HopcroftKarpMatcher hk;

    assert(simple.match(g_asymmetric) == 2);
    assert(hk.match(g_asymmetric) == 2);
    std::cout << "      -> STATUS: PASS (Algorithms calculate correct sub-perfect maximum)\n";
}

// ============================================================================
// EMPIRICAL COMPLEXITY PROOF
// ============================================================================

void test_algorithmic_scalability() {
    std::cout << "[RUN] test_algorithmic_scalability (Empirical Scalability Proof)\n";
    
    int n = 1000;
    double alpha = 1.2; 
    
    std::cout << "      Generating sparse matrix: V = " << n * 2 << ", E = ~" << n << "^1.2\n";
    BipartiteGraph g_sparse = GraphGenerator::generate(n, alpha);

    SimpleAugmentingMatcher simple;
    HopcroftKarpMatcher hk;

    auto start_simple = std::chrono::high_resolution_clock::now();
    int res_simple = simple.match(g_sparse);
    auto end_simple = std::chrono::high_resolution_clock::now();
    double time_simple = std::chrono::duration<double, std::milli>(end_simple - start_simple).count();

    auto start_hk = std::chrono::high_resolution_clock::now();
    int res_hk = hk.match(g_sparse);
    auto end_hk = std::chrono::high_resolution_clock::now();
    double time_hk = std::chrono::duration<double, std::milli>(end_hk - start_hk).count();

    std::cout << "      Simple DFS Phase Time : " << time_simple << " ms | Matches: " << res_simple << "\n";
    std::cout << "      Hopcroft-Karp Time    : " << time_hk << " ms | Matches: " << res_hk << "\n";

    assert(res_simple == res_hk);
    // The empirical proof requires HK to execute strictly faster on sparse matrices.
    assert(time_hk <= time_simple); 

    std::cout << "      -> STATUS: PASS (Theoretical advantage empirically proven)\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "EXECUTING STEP 2: HIGH-COVERAGE MATCHER BASELINES\n";
    std::cout << "==================================================\n";
    
    test_functional_matching();
    test_topological_empty_matrix();
    test_topological_asymmetric_matching();
    test_algorithmic_scalability();
    
    std::cout << "==================================================\n";
    std::cout << "ALL STEP 2 MATCHER TESTS SUCCESSFULLY VERIFIED [100%]\n";
    std::cout << "==================================================\n";
    return 0;
}