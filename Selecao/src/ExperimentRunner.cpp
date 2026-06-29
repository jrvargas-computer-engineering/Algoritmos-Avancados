#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "MetricsContext.h"
#include "NaiveSelection.h"
#include "RandomizedQuickSelect.h"
#include "MedianOfMedians.h"

// Generates specific array distributions to test algorithm robustness.
// Every (n, dist, seed) triple is deterministic, so the same instance is
// reproduced identically across all algorithms compared in a cell.
std::vector<int> generateData(int n, const std::string& dist, int seed) {
    std::vector<int> data(n);
    std::mt19937 rng(seed);

    if (dist == "uni") {
        // n DISTINCT values in random order. A random permutation of [0, n)
        // is distinct (the plan requires distinct keys, e.g. for E4's fit) and
        // comparison counts depend only on relative order, so the range is moot.
        std::iota(data.begin(), data.end(), 0);
        std::shuffle(data.begin(), data.end(), rng);
    }
    else if (dist == "gaussian") {
        // Floating values ~ N(0,1), scaled into the int domain. Realism check.
        std::normal_distribution<double> norm(0.0, 1.0);
        for (int i = 0; i < n; ++i) {
            data[i] = static_cast<int>(std::lround(norm(rng) * 100000.0));
        }
    }
    else if (dist == "alleq") {
        // Every element identical (Lomuto degeneracy stress).
        std::fill(data.begin(), data.end(), 42);
    }
    else if (dist == "sorted_asc") {
        std::iota(data.begin(), data.end(), 0);
    }
    else if (dist == "sorted_desc") {
        for (int i = 0; i < n; ++i) data[i] = n - i - 1;
    }
    else if (dist == "organpipe") {
        // Ascending then descending ("sawtooth" / organ pipe).
        int mid = n / 2;
        for (int i = 0; i < mid; ++i) data[i] = i;
        for (int i = mid; i < n; ++i) data[i] = n - i - 1;
    }
    else if (dist == "nearly_sorted") {
        // Sorted, then ~1% random pairwise swaps.
        std::iota(data.begin(), data.end(), 0);
        if (n > 1) {
            int swaps = std::max(1, n / 100);
            std::uniform_int_distribution<int> pick(0, n - 1);
            for (int s = 0; s < swaps; ++s) std::swap(data[pick(rng)], data[pick(rng)]);
        }
    }
    else if (dist == "dup_2") {
        std::uniform_int_distribution<int> uni(0, 1);
        for (int i = 0; i < n; ++i) data[i] = uni(rng);
    }
    else if (dist == "dup_5") {
        std::uniform_int_distribution<int> uni(0, 4);
        for (int i = 0; i < n; ++i) data[i] = uni(rng);
    }
    else {
        // Fail loudly: silently substituting uniform data would mislabel the
        // CSV (wrong-but-plausible rows) instead of surfacing the mistake.
        throw std::invalid_argument("Unknown distribution: " + dist);
    }

    return data;
}

// dist_param documented in the schema (m for dup_m, swap% for nearly_sorted).
// Returns "" when the distribution has no parameter.
std::string distParam(const std::string& dist) {
    if (dist == "dup_2") return "2";
    if (dist == "dup_5") return "5";
    if (dist == "nearly_sorted") return "1";
    return "";
}

std::string isoTimestamp() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&t));
    return std::string(buf);
}

int main(int argc, char* argv[]) {
    // Expected args:
    // [1] exp_id (e.g., E1)
    // [2] algorithm (naive, quickselect, mom, nth_element)
    // [3] n
    // [4] k
    // [5] g
    // [6] distribution (e.g., uni)
    // [7] seed / trial index
    if (argc < 8) {
        std::cerr << "Usage: run_experiments <exp_id> <algo> <n> <k> <g> <dist> <seed>\n";
        return 1;
    }

    std::string exp_id = argv[1];
    std::string algo_name = argv[2];
    int n = std::stoi(argv[3]);
    int k = std::stoi(argv[4]);
    int g = std::stoi(argv[5]);
    std::string dist = argv[6];
    int seed = std::stoi(argv[7]);

    // 1. Generate Data
    std::vector<int> data = generateData(n, dist, seed);

    // 2. Oracle Check (Always run Naive as the ground truth, §1.4)
    NaiveSelection<int> oracle;
    MetricsContext oracle_metrics;
    int expected_value = oracle.select(data, k, oracle_metrics);

    // 3. Setup Target Algorithm
    MetricsContext metrics;
    int result_value = 0;

    // Which side-channels are meaningful for this algorithm?
    bool has_accesses = (algo_name != "nth_element");          // not instrumentable for std::nth_element
    bool has_depth = (algo_name == "quickselect" || algo_name == "mom");
    bool has_algo_seed = (algo_name == "quickselect");          // only quickselect has internal RNG

    // Start Timer
    auto start_time = std::chrono::steady_clock::now();

    // Execute
    if (algo_name == "naive") {
        result_value = oracle.select(data, k, metrics);
    }
    else if (algo_name == "quickselect") {
        // Use the seed for the internal RNG of Quickselect to keep tests reproducible
        RandomizedQuickSelect<int> rqs(seed);
        result_value = rqs.select(data, k, metrics);
    }
    else if (algo_name == "mom") {
        MedianOfMediansSelection<int> mom(g);
        result_value = mom.select(data, k, metrics);
    }
    else if (algo_name == "nth_element") {
        // Investigating std::nth_element as requested by E9. Comparisons are
        // measurable via the comparator; swaps/accesses are not.
        std::vector<int> data_copy = data;
        std::nth_element(data_copy.begin(), data_copy.begin() + k, data_copy.end(),
            [&metrics](const int& a, const int& b) {
                metrics.comparisons++;
                return a < b;
            });
        result_value = data_copy[k];
    }
    else {
        std::cerr << "Unknown algorithm: " << algo_name << "\n";
        return 1;
    }

    // Stop Timer
    auto end_time = std::chrono::steady_clock::now();
    double time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

    bool oracle_match = (result_value == expected_value);

    // 4. Append to CSV
    // ofstream app-mode rows are small enough to flush as a single O_APPEND
    // write, which POSIX makes atomic -- safe for the parallel Phase A runs.
    std::ofstream csv_file("data/benchmark_data.csv", std::ios_base::app);
    if (csv_file.is_open()) {
        // Schema: run_id,experiment_id,algorithm,n,k,k_frac,g,distribution,dist_param,
        // instance_seed,algo_seed,trial_index,comparisons,accesses,recursion_depth,
        // time_ns,batch_size,build,result_value,oracle_match,timestamp
        csv_file << ","                                  // run_id (auto-increment skipped)
                 << exp_id << ","
                 << algo_name << ","
                 << n << ","
                 << k << ","
                 << (double)k / n << ","
                 << g << ","
                 << dist << ","
                 << distParam(dist) << ","               // dist_param
                 << seed << ",";                          // instance_seed
        if (has_algo_seed) csv_file << seed;              // algo_seed (null otherwise)
        csv_file << ","
                 << seed << ","                           // trial_index (rep index)
                 << metrics.comparisons << ",";
        if (has_accesses) csv_file << metrics.accesses;   // accesses (null for nth_element)
        csv_file << ",";
        if (has_depth) csv_file << metrics.recursion_depth; // recursion_depth (null for naive/nth_element)
        csv_file << ","
                 << time_ns << ","
                 << 1 << ","                              // batch_size
                 << "release,"                            // build
                 << result_value << ","
                 << (oracle_match ? "True" : "False") << ","
                 << isoTimestamp() << "\n";
    }

    // Surface oracle failures on stderr so a bad cell is noticed during the run.
    if (!oracle_match) {
        std::cerr << "ORACLE MISMATCH: " << exp_id << " " << algo_name << " n=" << n
                  << " k=" << k << " g=" << g << " dist=" << dist << " seed=" << seed
                  << " got=" << result_value << " expected=" << expected_value << "\n";
    }

    return 0;
}
