#include <iostream>
#include <cassert>
#include <sstream>
#include <memory>
#include "BipartiteGraph.h"
#include "GraphGenerator.h"
#include "IOHandler.h"
#include "HungarianMatcher.h"
#include "BellmanFordStrategy.h"

// ============================================================================
// NOMINAL RE-VERIFICATION
// ============================================================================

void test_weighted_matching_nominal() {
    std::cout << "[RUN] test_weighted_matching_nominal (Dense Positive Control)\n";
    
    std::string mock_input = 
        "3\n"
        "10 -5 20\n"
        "0 15 -10\n"
        "-2 4 8\n"
        "33\n";

    std::stringstream ss(mock_input);
    BipartiteGraph graph(1);
    int target_v = IOHandler::parse_input(ss, graph);

    auto strategy = std::make_shared<BellmanFordStrategy>();
    HungarianMatcher hungarian(strategy);

    int calculated_weight = hungarian.match(graph);
    std::cout << "      Expected: " << target_v << " | Actual: " << calculated_weight << "\n";
    assert(calculated_weight == target_v);
    std::cout << "      -> STATUS: PASS\n";
}

// ============================================================================
// STRICTLY NEGATIVE BOUNDARY EVALUATION
// ============================================================================

void test_weighted_matching_all_negative() {
    std::cout << "[RUN] test_weighted_matching_all_negative (Upper Negative Boundary)\n";
    
    // 2x2 matrix where all weights are negative. 
    // Max weight perfect matching must choose (-1) + (-4) = -5
    std::string mock_input = 
        "2\n"
        "-1 -10\n"
        "-20 -4\n"
        "-5\n";

    std::stringstream ss(mock_input);
    BipartiteGraph graph(1);
    int target_v = IOHandler::parse_input(ss, graph);

    auto strategy = std::make_shared<BellmanFordStrategy>();
    HungarianMatcher hungarian(strategy);

    int calculated_weight = hungarian.match(graph);
    std::cout << "      Expected: " << target_v << " | Actual: " << calculated_weight << "\n";
    assert(calculated_weight == target_v);
    std::cout << "      -> STATUS: PASS\n";
}

// ============================================================================
// SPARSE WEIGHTED MATRIX EVALUATION
// ============================================================================

void test_weighted_matching_sparse() {
    std::cout << "[RUN] test_weighted_matching_sparse (Missing Edge Bypassing)\n";
    
    int n = 10;
    double alpha = 1.3; // Generates a sparse configuration
    
    std::cout << "      Generating sparse weighted matrix via GraphGenerator (n=" << n << ")...\n";
    BipartiteGraph graph = GraphGenerator::generate(n, alpha);

    auto strategy = std::make_shared<BellmanFordStrategy>();
    HungarianMatcher hungarian(strategy);

    // This execution verifies that the shortest-path mechanics do not crash or 
    // enter infinite loops when navigating graphs containing structural voids.
    int calculated_weight = hungarian.match(graph);
    
    std::cout << "      Calculated Sparse Maximum Weight Sum: " << calculated_weight << "\n";
    // Structural validation: matching must complete, yielding an integer score
    assert(calculated_weight != IPathfindingStrategy::INF);
    std::cout << "      -> STATUS: PASS\n";
}

// ============================================================================
// MASTER ORCHESTRATOR
// ============================================================================

int main() {
    std::cout << "==================================================\n";
    std::cout << "EXECUTING STEP 3: HIGH-COVERAGE HUNGARIAN HARNESS\n";
    std::cout << "==================================================\n";
    
    test_weighted_matching_nominal();
    test_weighted_matching_all_negative();
    test_weighted_matching_sparse();
    
    std::cout << "==================================================\n";
    std::cout << "ALL STEP 3 ROBUSTNESS TESTS PASSED [100%]\n";
    std::cout << "==================================================\n";
    return 0;
}