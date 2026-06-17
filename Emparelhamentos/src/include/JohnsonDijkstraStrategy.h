#ifndef JOHNSON_DIJKSTRA_STRATEGY_H
#define JOHNSON_DIJKSTRA_STRATEGY_H

#include "IPathfindingStrategy.h"
#include <vector>

class JohnsonDijkstraStrategy : public IPathfindingStrategy {
private:
    std::vector<int> potentials;
    bool initialized = false;

    void initialize_potentials(const BipartiteGraph& g);

    #ifdef BENCHMARK_MODE
    private:
    long long dijkstra_call_count = 0;
    int potential_violations = 0;
    long long total_relaxations = 0;
    double accumulated_overhead_ms = 0.0;
    #endif

    public:
    bool find_shortest_augmenting_path(
        const BipartiteGraph& g, 
        const std::vector<int>& match_S, 
        const std::vector<int>& match_T, 
        std::vector<int>& predecessors,
        std::vector<int>& distances) override;

    #ifdef BENCHMARK_MODE
    void reset_metrics() override;
    #endif
    
    const std::vector<int>& get_potentials() const;
    int get_transformed_weight(const BipartiteGraph& g, int u, int v, bool is_forward) const;
};

#endif // JOHNSON_DIJKSTRA_STRATEGY_H