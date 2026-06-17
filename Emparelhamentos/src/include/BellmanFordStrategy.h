#ifndef BELLMAN_FORD_STRATEGY_H
#define BELLMAN_FORD_STRATEGY_H

#include "IPathfindingStrategy.h"

class BellmanFordStrategy : public IPathfindingStrategy {
public:
    bool find_shortest_augmenting_path(
        const BipartiteGraph& g, 
        const std::vector<int>& match_S, 
        const std::vector<int>& match_T, 
        std::vector<int>& predecessors,
        std::vector<int>& distances) override;
};

#endif // BELLMAN_FORD_STRATEGY_H