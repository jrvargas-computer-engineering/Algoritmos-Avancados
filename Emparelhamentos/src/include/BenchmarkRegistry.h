#ifndef BENCHMARK_REGISTRY_H
#define BENCHMARK_REGISTRY_H

#ifdef BENCHMARK_MODE
#include <string>
#include <ostream>
#include "CSVLogger.h"

struct BenchmarkRegistry {
    static std::ostream* out;
    static std::string test_id;
    static int n;
    static double alpha;
    static int rep;
    static std::string weight_regime;
    static std::string active_algorithm;
};

#define EMIT_METRIC(iteration, metric_name, metric_value) \
    if (BenchmarkRegistry::out) { \
        CSVLogger::log_metric(*BenchmarkRegistry::out, BenchmarkRegistry::test_id, \
            BenchmarkRegistry::n, BenchmarkRegistry::alpha, BenchmarkRegistry::rep, \
            BenchmarkRegistry::active_algorithm, iteration, BenchmarkRegistry::weight_regime, \
            metric_name, metric_value); \
    }
#else
// Zero-cost abstraction for the production binary
#define EMIT_METRIC(iteration, metric_name, metric_value) 
#endif

#endif // BENCHMARK_REGISTRY_H