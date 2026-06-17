#include "GraphGenerator.h"
#include <cmath>
#include <random>
#include <vector>
#include <numeric>
#include <algorithm>
#include <stdexcept>

BipartiteGraph GraphGenerator::generate(int n, double alpha, unsigned int seed) {
    if (n <= 0) {
        throw std::invalid_argument("Graph vertex parameter must be greater than zero.");
    }
    if (alpha < 1.0 || alpha > 2.0) {
        throw std::invalid_argument("Density exponent alpha must be within the closed interval [1.0, 2.0].");
    }

    BipartiteGraph graph(n);
    int target_edges = static_cast<int>(std::floor(std::pow(n, alpha)));
    int max_possible_edges = n * n;
    
    if (target_edges > max_possible_edges) {
        target_edges = max_possible_edges;
    }

    std::mt19937 rng(seed);
    std::vector<int> linear_indices(max_possible_edges);
    std::iota(linear_indices.begin(), linear_indices.end(), 0);
    std::shuffle(linear_indices.begin(), linear_indices.end(), rng);

    int min_weight = -(n * n);
    int max_weight = n * n;
    std::uniform_int_distribution<int> weight_dist(min_weight, max_weight);

    for (int i = 0; i < target_edges; ++i) {
        int idx = linear_indices[i];
        int u = idx / n;
        int v = idx % n;
        int weight = weight_dist(rng);
        graph.set_edge(u, v, weight);
    }

    return graph;
}