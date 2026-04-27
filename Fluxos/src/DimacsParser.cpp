#include "DimacsParser.hpp"
#include <sstream>
#include <stdexcept>
#include <vector>

// Helper struct to hold edge data before the network is instantiated
struct TempEdge {
    int u;
    int v;
    long long capacity;
};

FlowNetwork DimacsParser::parse_from_stream(std::istream& in) {
    std::string line;
    int num_nodes = 0;
    int num_edges_expected = 0;
    int source = -1;
    int sink = -1;
    
    std::vector<TempEdge> edges;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        char type;
        iss >> type;

        if (type == 'c') {
            // Comment line: ignore
            continue;
        } 
        else if (type == 'p') {
            // Problem description: p max <nodes> <edges>
            std::string problem_type;
            iss >> problem_type >> num_nodes >> num_edges_expected;
            if (problem_type != "max") {
                throw std::runtime_error("Parser error: Expected problem type 'max'");
            }
        } 
        else if (type == 'n') {
            // Node description: n <node_id> <s|t>
            int node_id;
            char node_role;
            iss >> node_id >> node_role;
            
            if (node_role == 's') {
                source = node_id - 1; // Convert 1-based to 0-based
            } else if (node_role == 't') {
                sink = node_id - 1;   // Convert 1-based to 0-based
            }
        } 
        else if (type == 'a') {
            // Arc/Edge description: a <u_id> <v_id> <capacity>
            int u, v;
            long long capacity;
            iss >> u >> v >> capacity;
            
            // Store temporarily, converting 1-based to 0-based
            edges.push_back({u - 1, v - 1, capacity});
        }
    }

    // Validation
    if (num_nodes == 0) {
        throw std::runtime_error("Parser error: Number of nodes is 0 or 'p' line missing.");
    }
    if (source == -1 || sink == -1) {
        throw std::runtime_error("Parser error: Source ('s') or Sink ('t') node not defined.");
    }

    // Construct the actual network
    FlowNetwork network(num_nodes, source, sink);
    for (const auto& edge : edges) {
        network.add_edge(edge.u, edge.v, edge.capacity);
    }

    return network;
}