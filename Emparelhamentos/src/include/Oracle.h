#ifndef ORACLE_H
#define ORACLE_H

#include "BipartiteGraph.h"

struct OracleResult {
    int max_size;
    int max_weight;
};

class Oracle {
public:
    // Evaluates the exact mathematical maximums via $O(n!)$ exhaustive enumeration
    static OracleResult evaluate(const BipartiteGraph& graph);

private:
    static void enumerate_subsets(const BipartiteGraph& graph, int u, int current_size, int current_weight, 
                                  std::vector<bool>& matched_T, OracleResult& best_result);
};

#endif // ORACLE_H