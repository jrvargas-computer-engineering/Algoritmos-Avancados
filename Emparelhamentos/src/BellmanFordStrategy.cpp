#include "BellmanFordStrategy.h"
#include <algorithm>

bool BellmanFordStrategy::find_shortest_augmenting_path(
    const BipartiteGraph& g, 
    const std::vector<int>& match_S, 
    const std::vector<int>& match_T, 
    std::vector<int>& predecessors,
    std::vector<int>& distances) 
{
    int n = g.get_n();
    distances.assign(n * 2, INF);
    predecessors.assign(n * 2, -1);

    // Initialize distances for all free vertices in S
    for (int u = 0; u < n; ++u) {
        if (match_S[u] == -1) {
            distances[u] = 0;
        }
    }

    bool relaxed = true;
    for (int step = 0; step < 2 * n && relaxed; ++step) {
        relaxed = false;
        for (int u = 0; u < n; ++u) {
            for (int v = 0; v < n; ++v) {
                if (g.has_edge(u, v)) {
                    int weight = g.get_weight(u, v);
                    
                    // Forward edge S -> T (not in matching)
                    if (match_S[u] != v) {
                        if (distances[u] != INF && distances[u] - weight < distances[n + v]) {
                            distances[n + v] = distances[u] - weight;
                            predecessors[n + v] = u;
                            relaxed = true;
                        }
                    }
                    // Reverse edge T -> S (in matching)
                    if (match_T[v] == u) {
                        if (distances[n + v] != INF && distances[n + v] + weight < distances[u]) {
                            distances[u] = distances[n + v] + weight;
                            predecessors[u] = n + v;
                            relaxed = true;
                        }
                    }
                }
            }
        }
    }

    // Identify the reachable free vertex in T with the minimum accumulated distance
    int min_dist = INF;
    int best_end_v = -1;
    for (int v = 0; v < n; ++v) {
        if (match_T[v] == -1 && distances[n + v] < min_dist) {
            min_dist = distances[n + v];
            best_end_v = n + v;
        }
    }

    return best_end_v != -1;
}