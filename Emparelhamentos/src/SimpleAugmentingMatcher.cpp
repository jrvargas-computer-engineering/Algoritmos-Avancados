#include "SimpleAugmentingMatcher.h"

bool SimpleAugmentingMatcher::dfs(int u, const BipartiteGraph& g, std::vector<bool>& visited, std::vector<int>& match_T) {
    int n = g.get_n();
    for (int v = 0; v < n; ++v) {
        if (g.has_edge(u, v) && !visited[v]) {
            visited[v] = true;
            // If vertex v in T is free, or its current match can find an alternate path
            if (match_T[v] < 0 || dfs(match_T[v], g, visited, match_T)) {
                match_T[v] = u;
                return true;
            }
        }
    }
    return false;
}

int SimpleAugmentingMatcher::match(const BipartiteGraph& g) {
    int n = g.get_n();
    std::vector<int> match_T(n, -1);
    int matching_size = 0;

    for (int u = 0; u < n; ++u) {
        std::vector<bool> visited(n, false);
        if (dfs(u, g, visited, match_T)) {
            matching_size++;
        }
    }
    return matching_size;
}