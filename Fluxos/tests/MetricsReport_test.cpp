#include <gtest/gtest.h>
#include "MaxFlowSolver.hpp"

// Test 1: Verify correct calculation of \bar{s} (vertices touched)
TEST(MetricsReportTest, CalculateSBar_NormalUse) {
    MetricsReport metrics;
    // Let's say we have 20 total vertices in the graph, and over 3 phases we touch:
    // Phase 1: 5 vertices (5/20 = 0.25)
    // Phase 2: 10 vertices (10/20 = 0.50)
    // Phase 3: 15 vertices (15/20 = 0.75)
    // Expected \bar{s} = (0.25 + 0.50 + 0.75) / 3 = 0.50
    
    metrics.vertices_touched_per_phase = {5, 10, 15};
    int total_vertices = 20;
    int total_phases = 3;

    EXPECT_DOUBLE_EQ(metrics.calculate_s_bar(total_vertices, total_phases), 0.50);
}

// Test 2: Verify correct calculation of \bar{t} (edges touched)
TEST(MetricsReportTest, CalculateTBar_NormalUse) {
    MetricsReport metrics;
    // 50 total edges. Over 2 phases:
    // Phase 1: 10 edges (10/50 = 0.2)
    // Phase 2: 20 edges (20/50 = 0.4)
    // Expected \bar{t} = (0.2 + 0.4) / 2 = 0.3
    
    metrics.edges_touched_per_phase = {10, 20};
    int total_edges = 50;
    int total_phases = 2;

    EXPECT_DOUBLE_EQ(metrics.calculate_t_bar(total_edges, total_phases), 0.30);
}

// Test 3: Guard against Divide-by-Zero
TEST(MetricsReportTest, HandlesZeroDivisorAndTotals) {
    MetricsReport metrics;
    metrics.vertices_touched_per_phase = {5, 10};
    metrics.edges_touched_per_phase = {5, 10};

    // If total phases (divisor) is 0, it should return 0.0 safely without crashing
    EXPECT_DOUBLE_EQ(metrics.calculate_s_bar(100, 0), 0.0);
    
    // If total vertices/edges are 0, it should return 0.0 safely
    EXPECT_DOUBLE_EQ(metrics.calculate_t_bar(0, 2), 0.0);
}