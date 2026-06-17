#ifndef IPATHFINDING_STRATEGY_H
#define IPATHFINDING_STRATEGY_H

#include "BipartiteGraph.h"
#include <vector>


class IPathfindingStrategy {
    public:
    // Shared theoretical upper bound for all shortest-path variants
    static constexpr int INF = 1e9; 
    
    virtual ~IPathfindingStrategy() = default;
    
    virtual bool find_shortest_augmenting_path(
        const BipartiteGraph& g, 
        const std::vector<int>& match_S, 
        const std::vector<int>& match_T, 
        std::vector<int>& predecessors,
        std::vector<int>& distances) = 0;
        #ifdef BENCHMARK_MODE
            virtual long long get_total_edge_relaxations() const { return 0; }
            virtual int get_potential_violations_count() const { return 0; }
            virtual double get_potential_update_overhead_ms() const { return 0.0; }
            virtual int get_dijkstra_calls() const { return 0; }
            virtual void reset_metrics() {}
        #endif
    };

#endif // IPATHFINDING_STRATEGY_H