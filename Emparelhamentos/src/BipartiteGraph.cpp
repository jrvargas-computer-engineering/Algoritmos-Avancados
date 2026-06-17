#include "BipartiteGraph.h"
#include <stdexcept>

BipartiteGraph::BipartiteGraph(int vertices_per_partition) {
    if (vertices_per_partition <= 0) {
        throw std::invalid_argument("Partition size must be strictly positive.");
    }
    n = vertices_per_partition;
    weight_matrix.assign(n, std::vector<int>(n, 0));
    edge_exists.assign(n, std::vector<bool>(n, false));
}

int BipartiteGraph::get_n() const { return n; }

void BipartiteGraph::set_edge(int u, int v, int weight) {
    if (u < 0 || u >= n || v < 0 || v >= n) {
        throw std::out_of_range("Vertex index out of bipartite partition bounds.");
    }
    weight_matrix[u][v] = weight;
    edge_exists[u][v] = true;
}

bool BipartiteGraph::has_edge(int u, int v) const {
    if (u < 0 || u >= n || v < 0 || v >= n) return false;
    return edge_exists[u][v];
}

int BipartiteGraph::get_weight(int u, int v) const {
    if (u < 0 || u >= n || v < 0 || v >= n || !edge_exists[u][v]) {
        return INF_NEGATIVE;
    }
    return weight_matrix[u][v];
}