#pragma once

#include "SelectionAlgorithm.h"
#include <algorithm>
#include <stdexcept>

template <typename T>
class NaiveSelection : public SelectionAlgorithm<T> {
public:
    T select(std::vector<T> data, int k, MetricsContext& metrics) override {
        // FIXED: Cast data.size() to int to safely compare with signed k
        if (k < 0 || k >= static_cast<int>(data.size())) {
            throw std::out_of_range("k is out of bounds");
        }

        std::sort(data.begin(), data.end(), [&metrics](const T& a, const T& b) {
            metrics.comparisons++;
            return a < b;
        });

        metrics.accesses++; 
        return data[k];
    }
};