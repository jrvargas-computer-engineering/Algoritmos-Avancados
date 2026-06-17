#ifndef SIMPLE_AUGMENTING_MATCHER_H
#define SIMPLE_AUGMENTING_MATCHER_H

#include "IMatcher.h"
#include <vector>

class SimpleAugmentingMatcher : public IMatcher {
private:
    bool dfs(int u, const BipartiteGraph& g, std::vector<bool>& visited, std::vector<int>& match_T);

public:
    int match(const BipartiteGraph& g) override;
};

#endif // SIMPLE_AUGMENTING_MATCHER_H