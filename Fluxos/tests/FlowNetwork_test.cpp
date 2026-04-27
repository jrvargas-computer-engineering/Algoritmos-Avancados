#include <gtest/gtest.h>
#include "FlowNetwork.hpp"

// Test 1: Verify correct initialization of the network
TEST(FlowNetworkTest, Initialization) {
    int num_nodes = 4;
    int source = 0;
    int sink = 3;
    FlowNetwork network(num_nodes, source, sink);

    EXPECT_EQ(network.get_num_vertices(), 4);
    EXPECT_EQ(network.get_num_edges(), 0);
    EXPECT_EQ(network.get_source(), 0);
    EXPECT_EQ(network.get_sink(), 3);
}

// Test 2: Verify that adding an edge correctly creates the forward and reverse edges
TEST(FlowNetworkTest, AddEdge) {
    FlowNetwork network(2, 0, 1);
    
    // Add an edge from node 0 to node 1 with capacity 10
    network.add_edge(0, 1, 10);

    EXPECT_EQ(network.get_num_edges(), 1);

    // Check forward edge (from 0 to 1)
    ASSERT_EQ(network.adj_list[0].size(), 1);
    Edge forward_edge = network.adj_list[0][0];
    EXPECT_EQ(forward_edge.to, 1);
    EXPECT_EQ(forward_edge.capacity, 10);
    EXPECT_EQ(forward_edge.flow, 0);
    EXPECT_EQ(forward_edge.rev_index, 0); // Should point to the first edge in adj_list[1]

    // Check reverse edge (from 1 to 0)
    ASSERT_EQ(network.adj_list[1].size(), 1);
    Edge reverse_edge = network.adj_list[1][0];
    EXPECT_EQ(reverse_edge.to, 0);
    EXPECT_EQ(reverse_edge.capacity, 0); // Reverse edge capacity is always 0 initially
    EXPECT_EQ(reverse_edge.flow, 0);
    EXPECT_EQ(reverse_edge.rev_index, 0); // Should point to the first edge in adj_list[0]
}

// Test 3: Verify that augmenting flow updates both forward and reverse edges correctly
TEST(FlowNetworkTest, AugmentFlow) {
    FlowNetwork network(2, 0, 1);
    network.add_edge(0, 1, 15);

    // Push 5 units of flow along the first edge of node 0
    network.augment_flow(0, 0, 5);

    Edge forward_edge = network.adj_list[0][0];
    Edge reverse_edge = network.adj_list[1][0];

    // Forward edge should have 5 flow, and 10 residual capacity
    EXPECT_EQ(forward_edge.flow, 5);
    EXPECT_EQ(network.get_residual_capacity(forward_edge), 10);

    // Reverse edge should have -5 flow, and 5 residual capacity (0 - (-5) = +5)
    EXPECT_EQ(reverse_edge.flow, -5);
    EXPECT_EQ(network.get_residual_capacity(reverse_edge), 5);
}

// Test 4: Verify that resetting flow clears all flow values back to zero
TEST(FlowNetworkTest, ResetFlow) {
    FlowNetwork network(3, 0, 2);
    network.add_edge(0, 1, 10);
    network.add_edge(1, 2, 10);

    // Push flow
    network.augment_flow(0, 0, 4);
    network.augment_flow(1, 1, 4);

    // Ensure flow is applied
    EXPECT_EQ(network.adj_list[0][0].flow, 4);
    EXPECT_EQ(network.adj_list[1][1].flow, 4);

    // Reset flow
    network.reset_flow();

    // Verify all flows are back to 0
    EXPECT_EQ(network.adj_list[0][0].flow, 0);
    EXPECT_EQ(network.adj_list[1][0].flow, 0);
    
    // Verify reverse edge flows are also back to 0
    EXPECT_EQ(network.adj_list[1][0].flow, 0); // reverse for 0->1
    EXPECT_EQ(network.adj_list[2][0].flow, 0); // reverse for 1->2
}