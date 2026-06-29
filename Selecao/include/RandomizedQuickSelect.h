#pragma once

#include "SelectionAlgorithm.h"
#include <vector>
#include <random>
#include <utility>
#include <stdexcept>

template <typename T>
class RandomizedQuickSelect : public SelectionAlgorithm<T> {
private:
    std::mt19937 rng; // Mersenne Twister random number generator

    T quickSelect(std::vector<T>& data, int left, int right, int k, MetricsContext& metrics, int depth = 1) {
        // Track the deepest level of the recursion reached.
        if (depth > metrics.recursion_depth) metrics.recursion_depth = depth;

        if (left == right) {
            metrics.accesses++;
            return data[left];
        }

        // 1. Randomly pick a pivot
        std::uniform_int_distribution<int> dist(left, right);
        int pivotIndex = dist(rng);

        // 2. Partition the array around the pivot
        pivotIndex = partition(data, left, right, pivotIndex, metrics);

        // 3. Recurse or return
        if (k == pivotIndex) {
            metrics.accesses++;
            return data[k];
        } else if (k < pivotIndex) {
            return quickSelect(data, left, pivotIndex - 1, k, metrics, depth + 1);
        } else {
            return quickSelect(data, pivotIndex + 1, right, k, metrics, depth + 1);
        }
    }

public:
    // Accept a seed for deterministic testing (TDD)
    explicit RandomizedQuickSelect(unsigned int seed = std::random_device{}()) : rng(seed) {}

    // Made public explicitly so we can test it independently
    int partition(std::vector<T>& data, int left, int right, int pivotIndex, MetricsContext& metrics) {
        T pivotValue = data[pivotIndex];
        metrics.accesses++; // Read pivotValue

        // Move pivot to the very end
        std::swap(data[pivotIndex], data[right]);
        metrics.accesses += 4; // std::swap = 2 reads + 2 writes

        int storeIndex = left;
        for (int i = left; i < right; i++) {
            metrics.comparisons++;
            metrics.accesses++; // Read data[i] for the comparison

            if (data[i] < pivotValue) {
                std::swap(data[storeIndex], data[i]);
                metrics.accesses += 4;
                storeIndex++;
            }
        }

        // Move pivot to its final sorted position
        std::swap(data[right], data[storeIndex]);
        metrics.accesses += 4;
        
        return storeIndex;
    }

    T select(std::vector<T> data, int k, MetricsContext& metrics) override {
        if (k < 0 || k >= static_cast<int>(data.size())) {
            throw std::out_of_range("k is out of bounds");
        }
        return quickSelect(data, 0, data.size() - 1, k, metrics);
    }
};