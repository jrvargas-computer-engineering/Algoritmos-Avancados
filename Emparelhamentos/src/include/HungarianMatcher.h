#ifndef HUNGARIAN_MATCHER_H
#define HUNGARIAN_MATCHER_H

#include "IMatcher.h"
#include "IPathfindingStrategy.h"
#include <memory>

class HungarianMatcher : public IMatcher {
private:
    std::shared_ptr<IPathfindingStrategy> strategy;

public:
    explicit HungarianMatcher(std::shared_ptr<IPathfindingStrategy> path_strategy);
    int match(const BipartiteGraph& g) override;
};

#endif // HUNGARIAN_MATCHER_H