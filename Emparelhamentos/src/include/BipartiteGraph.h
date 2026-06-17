#ifndef BIPARTITE_GRAPH_H
#define BIPARTITE_GRAPH_H

#include <vector>
#include <limits>

class BipartiteGraph {
private:
    int n;
    std::vector<std::vector<int>> weight_matrix;
    std::vector<std::vector<bool>> edge_exists;

public:
    static constexpr int INF_NEGATIVE = std::numeric_limits<int>::min();

    explicit BipartiteGraph(int vertices_per_partition);

    int get_n() const;
    void set_edge(int u, int v, int weight);
    bool has_edge(int u, int v) const;
    int get_weight(int u, int v) const;
};

#endif // BIPARTITE_GRAPH_H