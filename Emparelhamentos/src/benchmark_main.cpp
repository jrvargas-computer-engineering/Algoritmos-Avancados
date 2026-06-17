#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include "BipartiteGraph.h"
#include "GraphGenerator.h"
#include "CSVLogger.h"
#include "BenchmarkRegistry.h"
#include "SimpleAugmentingMatcher.h"
#include "HopcroftKarpMatcher.h"
#include "BellmanFordStrategy.h"
#include "JohnsonDijkstraStrategy.h"
#include "HungarianMatcher.h"
#include "Oracle.h"

// Define the static registry variables required by the telemetry macros
std::ostream* BenchmarkRegistry::out = nullptr;
std::string BenchmarkRegistry::test_id = "";
int BenchmarkRegistry::n = 0;
double BenchmarkRegistry::alpha = 0.0;
int BenchmarkRegistry::rep = 0;
std::string BenchmarkRegistry::weight_regime = "";
std::string BenchmarkRegistry::active_algorithm = "";

void apply_weight_regime(BipartiteGraph& g, int n, const std::string& regime, int rep) {
    int limit = n * n;
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (g.has_edge(u, v)) {
                // Pseudo-random but strictly deterministic weight generation
                int rand_val = (u * 31 + v * 17 + rep * 7) % (limit + 1);
                int w = 0;
                
                if (regime == "mixed") w = -limit + (rand_val % (2 * limit + 1));
                else if (regime == "negative") w = -limit + rand_val;
                else if (regime == "positive") w = rand_val;
                else w = rand_val; // default
                
                g.set_edge(u, v, w);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 6) return 1;

    // 1. Parameter Extraction
    std::string test_id = argv[1];
    int n = std::stoi(argv[2]);
    double alpha = std::stod(argv[3]);
    int rep = std::stoi(argv[4]);
    std::string weight_regime = argv[5];

    // 2. I/O Stream Initialization
    // Deterministic, collision-free name: one file per (test, n, alpha, rep, regime).
    // The raw CLI tokens keep it unique and idempotent across re-runs, which makes
    // parallel execution safe (no two invocations ever target the same file).
    std::string filename = "raw/" + test_id + "_n" + argv[2] + "_a" + argv[3]
                         + "_r" + argv[4] + "_" + weight_regime + ".csv";
    
    std::ofstream out_file(filename);
    if (!out_file.is_open()) return 1;
    CSVLogger::write_header(out_file);

    // Populate Registry for inner-loop macros
    BenchmarkRegistry::out = &out_file;
    BenchmarkRegistry::test_id = test_id;
    BenchmarkRegistry::n = n;
    BenchmarkRegistry::alpha = alpha;
    BenchmarkRegistry::rep = rep;
    BenchmarkRegistry::weight_regime = weight_regime;

    // 3. Topology Generation
    unsigned int seed = (n * 1000) + static_cast<int>(alpha * 100) + rep;
    BipartiteGraph graph = GraphGenerator::generate(n, alpha, seed);

    bool run_unweighted = (test_id[0] == 'U' || test_id == "C1");
    bool run_weighted = (test_id[0] == 'W' || test_id == "C2");

    // ========================================================================
    // ORACLE EXECUTION (GROUP 0)
    // ========================================================================
    if (test_id == "U0" || test_id == "W0") {
        if (test_id == "W0") apply_weight_regime(graph, n, weight_regime, rep);
        
        OracleResult truth = Oracle::evaluate(graph);
        BenchmarkRegistry::active_algorithm = "Oracle";
        EMIT_METRIC(-1, "oracle_size", truth.max_size);
        if (test_id == "W0") {
            EMIT_METRIC(-1, "oracle_value", truth.max_weight);
        }
    }

// ========================================================================
    // UNWEIGHTED ALGORITHMS (GROUP 1 & 3)
    // ========================================================================
    if (run_unweighted) {
        // Simple Augmenting
        BenchmarkRegistry::active_algorithm = "SimpleAugmentingMatcher";
        SimpleAugmentingMatcher simple;
        auto start = std::chrono::high_resolution_clock::now();
        int size_simple = simple.match(graph);
        auto end = std::chrono::high_resolution_clock::now();
        
        double elapsed_simple = std::chrono::duration<double, std::milli>(end - start).count();
        EMIT_METRIC(-1, "wall_clock_ms", elapsed_simple);
        EMIT_METRIC(-1, "matching_size", size_simple);

        // Hopcroft-Karp
        BenchmarkRegistry::active_algorithm = "HopcroftKarpMatcher";
        HopcroftKarpMatcher hk;
        start = std::chrono::high_resolution_clock::now();
        int size_hk = hk.match(graph);
        end = std::chrono::high_resolution_clock::now();
        
        double elapsed_hk = std::chrono::duration<double, std::milli>(end - start).count();
        EMIT_METRIC(-1, "wall_clock_ms", elapsed_hk);
        EMIT_METRIC(-1, "matching_size", size_hk);
    }

    // ========================================================================
    // WEIGHTED ALGORITHMS (GROUP 2 & 3)
    // ========================================================================
    if (run_weighted) {
        if (test_id != "W0") apply_weight_regime(graph, n, weight_regime, rep);

        // Bellman-Ford
        BenchmarkRegistry::active_algorithm = "HungarianMatcher+BellmanFordStrategy";
        auto bf_strategy = std::make_shared<BellmanFordStrategy>();
        HungarianMatcher hungarian_bf(bf_strategy);
        
        auto start = std::chrono::high_resolution_clock::now();
        hungarian_bf.match(graph);
        auto end = std::chrono::high_resolution_clock::now();
        
        double elapsed_bf = std::chrono::duration<double, std::milli>(end - start).count();
        EMIT_METRIC(-1, "wall_clock_ms", elapsed_bf);

        // Johnson-Dijkstra
        BenchmarkRegistry::active_algorithm = "HungarianMatcher+JohnsonDijkstraStrategy";
        auto jd_strategy = std::make_shared<JohnsonDijkstraStrategy>();
        HungarianMatcher hungarian_jd(jd_strategy);
        
        start = std::chrono::high_resolution_clock::now();
        hungarian_jd.match(graph);
        end = std::chrono::high_resolution_clock::now();
        
        double elapsed_jd = std::chrono::duration<double, std::milli>(end - start).count();
        EMIT_METRIC(-1, "wall_clock_ms", elapsed_jd);
    }

    out_file.close();
    return 0;
}