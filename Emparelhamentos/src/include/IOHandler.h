#ifndef IOHANDLER_H
#define IOHANDLER_H

#include "BipartiteGraph.h"
#include <iostream>
#include <stdexcept>

struct IOHandler {
    static int parse_input(std::istream& input, BipartiteGraph& graph) {
        int n;
        if (!(input >> n)) {
            throw std::runtime_error("Failed to parse partition dimension n from stream.");
        }
        if (n <= 0) {
            throw std::invalid_argument("Parsed partition dimension n must be positive.");
        }
        
        graph = BipartiteGraph(n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                int weight;
                if (!(input >> weight)) {
                    throw std::runtime_error("Stream corrupted or EOF reached prematurely during weight matrix extraction.");
                }
                graph.set_edge(i, j, weight);
            }
        }
        int target_v;
        if (!(input >> target_v)) {
            throw std::runtime_error("Failed to parse target perfect matching value v from stream.");
        }
        return target_v;
    }
};

#endif // IOHANDLER_H