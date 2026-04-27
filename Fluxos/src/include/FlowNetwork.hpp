#pragma once
#include <vector>

struct Edge {
    int to;
    long long capacity;
    long long flow;
    int rev_index;

    Edge(int to, long long capacity, int rev_index);
};

class FlowNetwork {
private:
    int num_vertices;
    int num_edges;
    int source_node;
    int sink_node;

public:
    std::vector<std::vector<Edge>> adj_list;

    FlowNetwork(int n, int s, int t);
    void add_edge(int u, int v, long long capacity);
    long long get_residual_capacity(const Edge& edge) const;
    void augment_flow(int u, int edge_index, long long amount);
    void reset_flow();

    int get_num_vertices() const;
    int get_num_edges() const;
    int get_source() const;
    int get_sink() const;
};