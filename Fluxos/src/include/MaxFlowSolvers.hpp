#pragma once
#include "MaxFlowSolver.hpp"


// Edmonds-Karp: Uses Shortest Path (BFS) to find augmenting paths.
class EdmondsKarp : public MaxFlowSolver {
public:
    MetricsReport solve(FlowNetwork& network) override;
};

// Ford-Fulkerson DFS: Uses Depth-First Search to find augmenting paths.
class FordFulkersonDFS : public MaxFlowSolver {
private:
    long long dfs(FlowNetwork& network, int u, int t, long long flow, 
                  std::vector<bool>& visited, int& v_touched, int& e_touched);
public:
    MetricsReport solve(FlowNetwork& network) override;
};

class FordFulkersonRandDFS : public MaxFlowSolver {
private:
    long long dfs(FlowNetwork& network, int u, int t, long long flow, 
                  std::vector<bool>& visited, int& v_touched, int& e_touched);
public:
    MetricsReport solve(FlowNetwork& network) override;
};

class FordFulkersonRandomDFS : public MaxFlowSolver {
public:
    MetricsReport solve(FlowNetwork& network) override; // Removed { return {}; }
};

class FattestPath : public MaxFlowSolver {
public:
    MetricsReport solve(FlowNetwork& network) override; // Removed { return {}; }
};

class CapacityScaling : public MaxFlowSolver {
public:
    MetricsReport solve(FlowNetwork& network) override; // Removed { return {}; }
};

class Dinitz : public MaxFlowSolver {
public:
    MetricsReport solve(FlowNetwork& network) override; // Removed { return {}; }
};