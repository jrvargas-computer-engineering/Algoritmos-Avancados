#include <gtest/gtest.h>
#include "MaxFlowSolvers.hpp"
#include "FlowNetwork.hpp"

class MaxFlowSolversTest : public ::testing::Test {
protected:
    // We can set up shared graphs here if we want, 
    // but building them inside the tests keeps them highly readable.
};

// Test 1: A simple straight line graph (S -> A -> T)
TEST_F(MaxFlowSolversTest, SimplePath) {
    FlowNetwork network(3, 0, 2);
    network.add_edge(0, 1, 15);
    network.add_edge(1, 2, 5); // Bottleneck is 5

    EdmondsKarp ek_solver;
    FordFulkersonDFS dfs_solver;

    // Test Edmonds-Karp
    MetricsReport ek_report = ek_solver.solve(network);
    EXPECT_EQ(ek_report.max_flow_value, 5);
    EXPECT_GT(ek_report.total_phases, 0); // Should have taken at least 1 phase

    // Reset the network to test the next algorithm
    network.reset_flow();

    // Test Ford-Fulkerson DFS
    MetricsReport dfs_report = dfs_solver.solve(network);
    EXPECT_EQ(dfs_report.max_flow_value, 5);
    EXPECT_GT(dfs_report.total_phases, 0);
}

// Test 2: The Classic Diamond Graph
TEST_F(MaxFlowSolversTest, DiamondGraph) {
    FlowNetwork network(4, 0, 3); // 4 nodes, Source=0, Sink=3
    
    // S -> A and S -> B
    network.add_edge(0, 1, 10);
    network.add_edge(0, 2, 10);
    
    // The trick edge: A -> B
    network.add_edge(1, 2, 1);
    
    // A -> T and B -> T
    network.add_edge(1, 3, 10);
    network.add_edge(2, 3, 10);

    EdmondsKarp ek_solver;
    FordFulkersonDFS dfs_solver;

    // 1. Run Edmonds-Karp
    MetricsReport ek_report = ek_solver.solve(network);
    EXPECT_EQ(ek_report.max_flow_value, 20);
    EXPECT_GT(ek_report.execution_time_ms, 0.0); // Ensure timer works
    EXPECT_FALSE(ek_report.vertices_touched_per_phase.empty()); // Ensure metrics tracked

    // 2. Reset and run Ford-Fulkerson
    network.reset_flow();
    MetricsReport dfs_report = dfs_solver.solve(network);
    EXPECT_EQ(dfs_report.max_flow_value, 20);
    EXPECT_GT(dfs_report.execution_time_ms, 0.0);
    EXPECT_FALSE(dfs_report.vertices_touched_per_phase.empty());
}

// Test 3: Disconnected Graph (No path from Source to Sink)
TEST_F(MaxFlowSolversTest, DisconnectedGraph) {
    FlowNetwork network(4, 0, 3);
    // S -> A
    network.add_edge(0, 1, 10);
    // B -> T
    network.add_edge(2, 3, 10);
    // Notice A and B are NOT connected.

    EdmondsKarp ek_solver;
    FordFulkersonDFS dfs_solver;

    MetricsReport ek_report = ek_solver.solve(network);
    EXPECT_EQ(ek_report.max_flow_value, 0); // Max flow must be 0

    network.reset_flow();
    
    MetricsReport dfs_report = dfs_solver.solve(network);
    EXPECT_EQ(dfs_report.max_flow_value, 0);
}