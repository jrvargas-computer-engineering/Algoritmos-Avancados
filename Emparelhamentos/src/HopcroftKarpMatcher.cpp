#include "HopcroftKarpMatcher.h"
#include "BenchmarkRegistry.h"

#include <queue>

bool HopcroftKarpMatcher::bfs(const BipartiteGraph& g, const std::vector<int>& match_S, const std::vector<int>& match_T, std::vector<int>& dist) {
    int n = g.get_n();
    std::queue<int> q;

    for (int u = 0; u < n; ++u) {
        if (match_S[u] < 0) {
            dist[u] = 0;
            q.push(u);
        } else {
            dist[u] = INF;
        }
    }
    
    // dist[n] acts as the distance to a generic free vertex in T
    dist[n] = INF; 

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        if (dist[u] < dist[n]) {
            for (int v = 0; v < n; ++v) {
                if (g.has_edge(u, v)) {

                #ifdef BENCHMARK_MODE
                current_phase_bfs_edges++;
                #endif

                    if (match_T[v] < 0) {
                        dist[n] = dist[u] + 1;
                    } else if (dist[match_T[v]] == INF) {
                        dist[match_T[v]] = dist[u] + 1;
                        q.push(match_T[v]);
                    }
                }
            }
        }
    }
    return dist[n] != INF;
}

bool HopcroftKarpMatcher::dfs(int u, const BipartiteGraph& g, std::vector<int>& match_S, std::vector<int>& match_T, std::vector<int>& dist) {
    int n = g.get_n();
    if (u != n) {
        for (int v = 0; v < n; ++v) {
            if (g.has_edge(u, v)) {


                #ifdef BENCHMARK_MODE
                current_phase_dfs_edges++;
                #endif
                
                if (match_T[v] < 0 || dist[match_T[v]] == dist[u] + 1) {
                    if (match_T[v] < 0 || dfs(match_T[v], g, match_S, match_T, dist)) {
                        match_T[v] = u;
                        match_S[u] = v;
                        return true;
                    }
                }
            }
        }
        dist[u] = INF;
        return false;
    }
    return true;
}

int HopcroftKarpMatcher::match(const BipartiteGraph& g) {
    int n = g.get_n();
    std::vector<int> match_S(n, -1);
    std::vector<int> match_T(n, -1);
    std::vector<int> dist(n + 1, INF);
    

    #ifdef BENCHMARK_MODE
    BenchmarkRegistry::active_algorithm = "HopcroftKarpMatcher";
    int phase_index = 0;
    long long total_bfs_edges = 0;
    long long total_dfs_edges = 0;
    current_phase_bfs_edges = 0; // Ensure initial reset
    current_phase_dfs_edges = 0;
    #endif
    
    int matching_size = 0;
    // CORRECTION 1: Pass all 4 required arguments to bfs()
    while (bfs(g, match_S, match_T, dist)) {
        int paths_found = 0;
        for (int u = 0; u < n; ++u) {
            
            // CORRECTION 2: Pass all 5 required arguments to dfs() in the correct order
            if (match_S[u] == -1 && dfs(u, g, match_S, match_T, dist)) {
                paths_found++;
                matching_size++;
            }
        }
        if (paths_found == 0) break;

        #ifdef BENCHMARK_MODE
        int current_defect = n - matching_size;
        EMIT_METRIC(phase_index, "paths_found_per_phase", paths_found);
        EMIT_METRIC(phase_index, "bfs_edges_visited", current_phase_bfs_edges); 
        EMIT_METRIC(phase_index, "dfs_edges_visited", current_phase_dfs_edges); 
        EMIT_METRIC(phase_index, "defect_at_iteration", current_defect);
        
        total_bfs_edges += current_phase_bfs_edges;
        total_dfs_edges += current_phase_dfs_edges;
        current_phase_bfs_edges = 0;
        current_phase_dfs_edges = 0;
        phase_index++;
        #endif
    }

    #ifdef BENCHMARK_MODE
    EMIT_METRIC(-1, "phase_count", phase_index);
    EMIT_METRIC(-1, "main_loop_iters", phase_index);
    EMIT_METRIC(-1, "total_bfs_edges_visited", total_bfs_edges);
    EMIT_METRIC(-1, "total_edges_visited", total_bfs_edges + total_dfs_edges);
    EMIT_METRIC(-1, "matching_size", matching_size);
    #endif

    return matching_size;
}