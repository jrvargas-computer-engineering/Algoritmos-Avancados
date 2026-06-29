#pragma once

#include "SelectionAlgorithm.h"
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>

template <typename T>
class MedianOfMediansSelection : public SelectionAlgorithm<T> {
private:
    int g; // Group size (e.g., 3, 5, 7)

    // Reusing Lomuto partition logic from Phase 2
    int partition(std::vector<T>& data, int left, int right, int pivotIndex, MetricsContext& metrics) {
        T pivotValue = data[pivotIndex];
        metrics.accesses++;

        std::swap(data[pivotIndex], data[right]);
        metrics.accesses += 4;

        int storeIndex = left;
        for (int i = left; i < right; i++) {
            metrics.comparisons++;
            metrics.accesses++;
            if (data[i] < pivotValue) {
                std::swap(data[storeIndex], data[i]);
                metrics.accesses += 4;
                storeIndex++;
            }
        }
        std::swap(data[right], data[storeIndex]);
        metrics.accesses += 4;
        return storeIndex;
    }

    T momSelect(std::vector<T>& data, int left, int right, int k, MetricsContext& metrics, int depth = 1) {
        // Track the deepest level reached (counts the nested median-of-medians
        // recursion via get_pivot too, which is what makes g=3 degrade).
        if (depth > metrics.recursion_depth) metrics.recursion_depth = depth;

        int n = right - left + 1;

        // Base case: if the segment is smaller than or equal to group size, just sort and return
        if (n <= g) {
            std::sort(data.begin() + left, data.begin() + right + 1, [&metrics](const T& a, const T& b) {
                metrics.comparisons++;
                return a < b;
            });
            metrics.accesses++; // Final access
            return data[k];
        }

        // 1. Find deterministic pivot
        T pivotValue = get_pivot(data, left, right, metrics, depth);

        // 2. Find the index of the pivot value in the current segment
        int pivotIndex = left;
        for (int i = left; i <= right; i++) {
            metrics.accesses++;
            metrics.comparisons++;
            if (data[i] == pivotValue) {
                pivotIndex = i;
                break;
            }
        }

        // 3. Partition around the pivot
        pivotIndex = partition(data, left, right, pivotIndex, metrics);

        // 4. Recurse or return
        if (k == pivotIndex) {
            metrics.accesses++;
            return data[k];
        } else if (k < pivotIndex) {
            return momSelect(data, left, pivotIndex - 1, k, metrics, depth + 1);
        } else {
            return momSelect(data, pivotIndex + 1, right, k, metrics, depth + 1);
        }
    }

public:
    explicit MedianOfMediansSelection(int group_size = 5) : g(group_size) {
        if (g < 1) throw std::invalid_argument("Group size must be >= 1");
    }

    // Exposed for TDD
    T get_median_of_small_group(std::vector<T>& data, int left, int right, MetricsContext& metrics) {
        std::sort(data.begin() + left, data.begin() + right + 1, [&metrics](const T& a, const T& b) {
            metrics.comparisons++;
            return a < b;
        });
        int medianIndex = left + (right - left) / 2;
        metrics.accesses++;
        return data[medianIndex];
    }

    // Exposed for TDD. `depth` defaults to 1 so the public/test call site
    // (and any caller that doesn't track depth) still compiles.
    T get_pivot(std::vector<T>& data, int left, int right, MetricsContext& metrics, int depth = 1) {
        int n = right - left + 1;
        std::vector<T> medians;

        // Divide into groups of size g and find the median of each
        for (int i = 0; i < n; i += g) {
            int subRight = std::min(left + i + g - 1, right);
            T med = get_median_of_small_group(data, left + i, subRight, metrics);
            medians.push_back(med);
        }

        // Recursively find the median of the medians
        // (Note: medians is a new vector, so k is medians.size() / 2, relative to 0)
        return momSelect(medians, 0, medians.size() - 1, medians.size() / 2, metrics, depth + 1);
    }

    T select(std::vector<T> data, int k, MetricsContext& metrics) override {
        if (k < 0 || k >= static_cast<int>(data.size())) {
            throw std::out_of_range("k is out of bounds");
        }
        return momSelect(data, 0, data.size() - 1, k, metrics);
    }
};