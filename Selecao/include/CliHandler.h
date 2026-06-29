#pragma once
#include <iostream>
#include <vector>
#include "SelectionAlgorithm.h"
#include "MetricsContext.h"

// Extracts the I/O logic so we can test it with stringstreams
template <typename T>
void processInput(std::istream& in, std::ostream& out, SelectionAlgorithm<T>& algo) {
    int n;
    // 1. Read n
    if (!(in >> n)) return;

    // 2. Read n elements
    std::vector<T> data(n);
    for (int i = 0; i < n; ++i) {
        in >> data[i];
    }

    // 3. Read k
    int k;
    if (!(in >> k)) return;

    // Execute the algorithm
    MetricsContext metrics;
    T result = algo.select(data, k, metrics);
    
    // 4. Output the k-th smallest element
    out << result << "\n";
}