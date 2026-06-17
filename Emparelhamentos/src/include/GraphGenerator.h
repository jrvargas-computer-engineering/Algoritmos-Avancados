#ifndef GRAPH_GENERATOR_H
#define GRAPH_GENERATOR_H

#include "BipartiteGraph.h"

class GraphGenerator {
public:
    static BipartiteGraph generate(int n, double alpha, unsigned int seed = 1337);
};

#endif // GRAPH_GENERATOR_H