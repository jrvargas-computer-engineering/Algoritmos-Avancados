#include "include/FlowNetwork.hpp"

// Edge Constructor
Edge::Edge(int to, long long capacity, int rev_index) 
    : to(to), capacity(capacity), flow(0), rev_index(rev_index) {}

// FlowNetwork Constructor
FlowNetwork::FlowNetwork(int n, int s, int t) 
    : num_vertices(n), num_edges(0), source_node(s), sink_node(t) {
    adj_list.resize(n);
}

void FlowNetwork::add_edge(int u, int v, long long capacity) {
    int rev_idx_for_u = adj_list[v].size();
    int rev_idx_for_v = adj_list[u].size();
    if (u == v) rev_idx_for_u++;

    adj_list[u].emplace_back(v, capacity, rev_idx_for_u);
    adj_list[v].emplace_back(u, 0, rev_idx_for_v);
    num_edges++;
}

long long FlowNetwork::get_residual_capacity(const Edge& edge) const {
    return edge.capacity - edge.flow;
}

void FlowNetwork::augment_flow(int u, int edge_index, long long amount) {
    Edge& forward_edge = adj_list[u][edge_index];
    Edge& reverse_edge = adj_list[forward_edge.to][forward_edge.rev_index];
    
    forward_edge.flow += amount;
    reverse_edge.flow -= amount; 
}

void FlowNetwork::reset_flow() {
    for (int u = 0; u < num_vertices; ++u) {
        for (auto& edge : adj_list[u]) {
            edge.flow = 0;
        }
    }
}

int FlowNetwork::get_num_vertices() const { return num_vertices; }
int FlowNetwork::get_num_edges() const { return num_edges; }
int FlowNetwork::get_source() const { return source_node; }
int FlowNetwork::get_sink() const { return sink_node; }