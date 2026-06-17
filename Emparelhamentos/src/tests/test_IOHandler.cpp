#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <stdexcept>
#include "BipartiteGraph.h"
#include "IOHandler.h"

void test_BipartiteGraph_mechanics();
void test_BipartiteGraph_exceptional();
void test_GraphGenerator_constraints();
void test_GraphGenerator_exceptional();

void test_IOHandler_parsing() {
    std::cout << "[RUN] test_IOHandler_parsing (Nominal)\n";
    std::string mock_input = "2\n5 10\n-3 2\n8\n";
    std::stringstream ss(mock_input);
    BipartiteGraph graph(1);
    
    int target_v = IOHandler::parse_input(ss, graph);
    assert(graph.get_n() == 2);
    assert(target_v == 8);
    assert(graph.get_weight(0, 0) == 5);
    std::cout << "      -> STATUS: PASS\n";
}

void test_IOHandler_exceptional() {
    std::cout << "[RUN] test_IOHandler_exceptional (Stream Fault Injection)\n";

    std::string truncated_input = "3\n10 20 30\n40";
    std::stringstream ss1(truncated_input);
    BipartiteGraph g1(1);
    try {
        IOHandler::parse_input(ss1, g1);
        assert(false && "Failed to intercept truncated stream.");
    } catch (const std::runtime_error& e) {
        std::cout << "      Caught expected exception for premature EOF condition.\n";
    }

    std::string corrupted_input = "2\n1 2\n3 CORRUPT\n9\n";
    std::stringstream ss2(corrupted_input);
    BipartiteGraph g2(1);
    try {
        IOHandler::parse_input(ss2, g2);
        assert(false && "Failed to intercept alphanumeric data contamination.");
    } catch (const std::runtime_error& e) {
        std::cout << "      Caught expected exception for corrupted type injection.\n";
    }

    std::string negative_dim_input = "-2\n5 6\n7 8\n10\n";
    std::stringstream ss3(negative_dim_input);
    BipartiteGraph g3(1);
    try {
        IOHandler::parse_input(ss3, g3);
        assert(false && "Failed to intercept negative dimensions matrix input.");
    } catch (const std::invalid_argument& e) {
        std::cout << "      Caught expected exception for parsed negative dimension parameter.\n";
    }

    std::cout << "      -> STATUS: PASS\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "EXECUTING EXPANDED HIGH-COVERAGE FAULT-TOLERANT HARNESS\n";
    std::cout << "==================================================\n";
    
    test_BipartiteGraph_mechanics();
    test_BipartiteGraph_exceptional();
    
    test_GraphGenerator_constraints();
    test_GraphGenerator_exceptional();
    
    test_IOHandler_parsing();
    test_IOHandler_exceptional();
    
    std::cout << "==================================================\n";
    std::cout << "ALL COMPREHENSIVE COVERAGE TESTS PASSED [100%]\n";
    std::cout << "==================================================\n";
    return 0;
}