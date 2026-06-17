#include <iostream>
#include <string>
#include <chrono>
#include <cmath>
#include <memory>
#include <vector>
#include "BipartiteGraph.h"
#include "GraphGenerator.h"
#include "SimpleAugmentingMatcher.h"
#include "HopcroftKarpMatcher.h"
#include "HungarianMatcher.h"
#include "BellmanFordStrategy.h"
#include "JohnsonDijkstraStrategy.h"

int main(int argc, char* argv[]) {
    // 1. Parameter Extraction
    if (argc != 3) {
        std::cerr << "Usage: ./performance_worker <n> <alpha>\n";
        return 1;
    }

    int n = std::stoi(argv[1]);
    double alpha = std::stod(argv[2]);
    int expected_edges = static_cast<int>(std::floor(std::pow(n, alpha)));
    unsigned int stable_seed = 20260610; // Guarantees identical topologies across algorithm runs

    // 2. Topology Generation
    BipartiteGraph graph = GraphGenerator::generate(n, alpha, stable_seed);

    // 3. Matcher Initialization
    SimpleAugmentingMatcher simple_matcher;
    HopcroftKarpMatcher hk_matcher;
    auto bf_strategy = std::make_shared<BellmanFordStrategy>();
    auto jd_strategy = std::make_shared<JohnsonDijkstraStrategy>();
    HungarianMatcher hungarian_bf(bf_strategy);
    HungarianMatcher hungarian_jd(jd_strategy);

    // 4. Execution & Telemetry Streaming
    // Format: Algorithm,N,Alpha,Edges,MatchingResult,OperationsCount,ExecutionTimeMS

    // --- Simple Augmenting (Control Baseline) ---
    auto start = std::chrono::high_resolution_clock::now();
    int res_simple = simple_matcher.match(graph);
    auto end = std::chrono::high_resolution_clock::now();
    double time_simple = std::chrono::duration<double, std::milli>(end - start).count();
    long long ops_simple = static_cast<long long>(n) * expected_edges;
    std::cout << "SimpleAugmenting," << n << "," << alpha << "," << expected_edges << "," 
              << res_simple << "," << ops_simple << "," << time_simple << "\n";

    // --- Hopcroft-Karp (Unweighted Maximum) ---
    start = std::chrono::high_resolution_clock::now();
    int res_hk = hk_matcher.match(graph);
    end = std::chrono::high_resolution_clock::now();
    double time_hk = std::chrono::duration<double, std::milli>(end - start).count();
    long long ops_hk = static_cast<long long>(std::sqrt(n)) * expected_edges;
    std::cout << "HopcroftKarp," << n << "," << alpha << "," << expected_edges << "," 
              << res_hk << "," << ops_hk << "," << time_hk << "\n";

    // --- Hungarian Bellman-Ford (Weighted Control) ---
    start = std::chrono::high_resolution_clock::now();
    int res_bf = hungarian_bf.match(graph);
    end = std::chrono::high_resolution_clock::now();
    double time_bf = std::chrono::duration<double, std::milli>(end - start).count();
    long long ops_bf = static_cast<long long>(2 * n) * expected_edges * n;
    std::cout << "Hungarian_BellmanFord," << n << "," << alpha << "," << expected_edges << "," 
              << res_bf << "," << ops_bf << "," << time_bf << "\n";

    // --- Hungarian Johnson-Dijkstra (Weighted Maximum) ---
    start = std::chrono::high_resolution_clock::now();
    int res_jd = hungarian_jd.match(graph);
    end = std::chrono::high_resolution_clock::now();
    double time_jd = std::chrono::duration<double, std::milli>(end - start).count();
    long long ops_jd = static_cast<long long>(expected_edges * std::log2(2 * n)) * n;
    std::cout << "Hungarian_JohnsonDijkstra," << n << "," << alpha << "," << expected_edges << "," 
              << res_jd << "," << ops_jd << "," << time_jd << "\n";

    return 0;
}