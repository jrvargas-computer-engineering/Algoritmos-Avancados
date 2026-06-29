#pragma once

#include <vector>
#include "MetricsContext.h"

// Abstract base class for the Strategy Pattern
template <typename T>
class SelectionAlgorithm {
public:
    virtual ~SelectionAlgorithm() = default;
    
    // Pass by value (std::vector<T> data) is intentional here because 
    // selection algorithms often mutate the array (e.g., partitioning), 
    // and we want to preserve the original array for fair testing.
    virtual T select(std::vector<T> data, int k, MetricsContext& metrics) = 0;
};