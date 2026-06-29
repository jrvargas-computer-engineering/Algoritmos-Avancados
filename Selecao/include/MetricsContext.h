#pragma once

// Tracks empirical metrics for algorithmic analysis
struct MetricsContext {
    long long comparisons = 0;
    long long accesses = 0;
    // Maximum recursion depth reached during the selection (1 = no recursion).
    // Instrumented for MoM and Quickselect; stays 0 for the non-recursive
    // NaiveSelection and for the un-instrumentable std::nth_element.
    long long recursion_depth = 0;

    void reset() {
        comparisons = 0;
        accesses = 0;
        recursion_depth = 0;
    }
};
