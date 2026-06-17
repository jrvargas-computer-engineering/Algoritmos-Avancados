#include <iostream>
#include <cassert>
#include <stdexcept>
#include "BipartiteGraph.h"

void test_BipartiteGraph_mechanics() {
    std::cout << "[RUN] test_BipartiteGraph_mechanics (Nominal)\n";
    BipartiteGraph graph(5);
    assert(graph.get_n() == 5);
    
    graph.set_edge(1, 2, 45);
    assert(graph.has_edge(1, 2) == true);
    assert(graph.get_weight(1, 2) == 45);
    std::cout << "      -> STATUS: PASS\n";
}

void test_BipartiteGraph_exceptional() {
    std::cout << "[RUN] test_BipartiteGraph_exceptional (Out of Bounds & Degenerate)\n";
    
    try {
        BipartiteGraph graph_invalid(0);
        assert(false && "Failed to intercept zero partition size initialization.");
    } catch (const std::invalid_argument& e) {
        std::cout << "      Caught expected exception for zero-size graph initialization.\n";
    }

    BipartiteGraph graph(3);
    
    try {
        graph.set_edge(3, 1, 10);
        assert(false && "Failed to intercept out-of-bounds row index.");
    } catch (const std::out_of_range& e) {
        std::cout << "      Caught expected exception for upper bound vertex breach.\n";
    }

    try {
        graph.set_edge(1, -1, 10);
        assert(false && "Failed to intercept negative column index.");
    } catch (const std::out_of_range& e) {
        std::cout << "      Caught expected exception for negative vertex index breach.\n";
    }

    assert(graph.has_edge(5, 5) == false);
    assert(graph.get_weight(-1, 2) == BipartiteGraph::INF_NEGATIVE);
    
    std::cout << "      -> STATUS: PASS\n";
}