#include <iostream>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include "BipartiteGraph.h"
#include "GraphGenerator.h"

void test_GraphGenerator_constraints() {
    std::cout << "[RUN] test_GraphGenerator_constraints (Nominal)\n";
    int n = 10;
    double alpha = 1.5;
    int expected_edges = static_cast<int>(std::floor(std::pow(n, alpha)));
    
    BipartiteGraph graph = GraphGenerator::generate(n, alpha);
    int actual_edges = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (graph.has_edge(i, j)) actual_edges++;
        }
    }
    assert(actual_edges == expected_edges);
    std::cout << "      -> STATUS: PASS\n";
}

void test_GraphGenerator_exceptional() {
    std::cout << "[RUN] test_GraphGenerator_exceptional (Invalid Parameters)\n";
    
    try {
        GraphGenerator::generate(-5, 1.5);
        assert(false && "Failed to intercept negative dimension parameter.");
    } catch (const std::invalid_argument& e) {
        std::cout << "      Caught expected exception for negative size vector allocation.\n";
    }

    try {
        GraphGenerator::generate(10, 0.9);
        assert(false && "Failed to intercept sub-unity alpha exponent.");
    } catch (const std::invalid_argument& e) {
        std::cout << "      Caught expected exception for sub-unity density exponent.\n";
    }

    try {
        GraphGenerator::generate(10, 2.1);
        assert(false && "Failed to intercept supra-quadratic alpha exponent.");
    } catch (const std::invalid_argument& e) {
        std::cout << "      Caught expected exception for supra-quadratic density exponent.\n";
    }
    
    std::cout << "      -> STATUS: PASS\n";
}