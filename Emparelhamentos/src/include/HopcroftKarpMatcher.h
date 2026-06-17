#ifndef HOPCROFT_KARP_MATCHER_H
#define HOPCROFT_KARP_MATCHER_H

#include "IMatcher.h"
#include <vector>

class HopcroftKarpMatcher : public IMatcher {
private:
    static constexpr int INF = 1e9;
    bool bfs(const BipartiteGraph& g, const std::vector<int>& match_S, const std::vector<int>& match_T, std::vector<int>& dist);
    bool dfs(int u, const BipartiteGraph& g, std::vector<int>& match_S, std::vector<int>& match_T, std::vector<int>& dist);

    #ifdef BENCHMARK_MODE
    long long current_phase_bfs_edges = 0;
    long long current_phase_dfs_edges = 0;
    #endif

public:
    int match(const BipartiteGraph& g) override;
};

#endif // HOPCROFT_KARP_MATCHER_H