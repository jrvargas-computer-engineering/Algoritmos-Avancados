#ifndef SPARSEGRAPH_HPP   
#define SPARSEGRAPH_HPP

#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>

struct Edge {
    int to;
    int weight;
};

class SparseGraph {
private:
    int numVertices;
    // Each index in the vector represents a vertex
    // Each list contains the edges originating from that vertex
    std::vector<std::list<Edge>> adj;

public:
    // Constructor
    SparseGraph(int vertices) : numVertices(vertices) {
        adj.resize(vertices);
    }

    // Add a directed edge
    void addEdge(int from, int to, int weight) {
        if (from >= 0 && from < numVertices) {
            adj[from].push_back({to, weight});
        }
    }

    // Getter for Dijkstra to use
    const std::list<Edge>& getNeighbors(int u) const {
        return adj[u];
    }

    int getNumVertices() const {
        return numVertices;
    }

    // --- DIMACS STDIN READER METHOD (ROBUST) ---
    static SparseGraph fromDIMACS() {
        std::string line;
        int numVertices = 0;
        int expectedEdges = 0;

        // Phase 1: Search the file for the 'p' line to get graph dimensions
        while (std::getline(std::cin, line)) {
            std::istringstream iss(line);
            std::string type;
            
            // Extract the first token safely. If the line is empty/whitespace, skip it.
            if (!(iss >> type)) continue; 

            if (type == "p") {
                std::string sp;
                iss >> sp >> numVertices >> expectedEdges;
                break; // We have the dimensions, move to Phase 2
            }
        }

        if (numVertices == 0) {
            throw std::runtime_error("Invalid DIMACS format: missing 'p' line.");
        }

        SparseGraph g(numVertices);
        int parsedEdges = 0;

        // Phase 2: Read the rest of the file for 'a' (arc/edge) lines
        while (std::getline(std::cin, line)) {
            std::istringstream iss(line);
            std::string type;
            
            if (!(iss >> type)) continue;

            if (type == "a") {
                int from, to, weight;
                iss >> from >> to >> weight;
                
                // Subtract 1 to translate DIMACS 1-indexing to C++ 0-indexing
                g.addEdge(from - 1, to - 1, weight);
                parsedEdges++;
            }
        }

        // Print to standard error (ignored by grading scripts) so you can verify!
        std::cerr << "[DEBUG] Loaded " << numVertices << " vertices and " 
                  << parsedEdges << " edges.\n";

        return g;
    }
};

#endif