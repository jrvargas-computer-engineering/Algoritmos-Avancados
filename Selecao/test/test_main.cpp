#include "MetricsContext.h"
#include "NaiveSelection.h"
#include "RandomizedQuickSelect.h"
#include "MedianOfMedians.h"
#include "CliHandler.h"
#include <sstream>
#include <iostream>
#include <cassert>
#include <vector>

void testMetricsContext() {
    MetricsContext metrics;
    assert(metrics.comparisons == 0);
    metrics.comparisons += 5;
    metrics.reset();
    assert(metrics.comparisons == 0);
    std::cout << "testMetricsContext passed.\n";
}

template <typename T>
void runSelectionTests(SelectionAlgorithm<T>& algorithm, const std::string& algoName) {
    MetricsContext metrics;

    std::vector<int> data1 = {7, 10, 4, 3, 20, 15};
    assert(algorithm.select(data1, 0, metrics) == 3); // Minimum
    metrics.reset();
    assert(algorithm.select(data1, 5, metrics) == 20); // Maximum
    metrics.reset();
    assert(algorithm.select(data1, 3, metrics) == 10); // Median

    std::vector<int> data2 = {5, 5, 5, 2, 2, 9, 1}; // Duplicates
    metrics.reset();
    assert(algorithm.select(data2, 3, metrics) == 5);

    std::vector<int> data3 = {1, 2, 3, 4, 5, 6, 7}; // Already sorted
    metrics.reset();
    assert(algorithm.select(data3, 2, metrics) == 3);

    std::vector<int> data4 = {7, 6, 5, 4, 3, 2, 1}; // Reverse sorted
    metrics.reset();
    assert(algorithm.select(data4, 0, metrics) == 1);

    std::cout << algoName << " edge-case tests passed.\n";
}

void testQuickSelectPartition() {
    RandomizedQuickSelect<int> rqs;
    MetricsContext metrics;
    std::vector<int> data = {9, 2, 7, 3, 5, 8, 4}; // Pivot will be 5 (index 4)
    
    int newPivotIndex = rqs.partition(data, 0, data.size() - 1, 4, metrics);
    
    // 5 should end up at index 3. 
    // Elements < 5 should be on the left; elements >= 5 on the right.
    assert(newPivotIndex == 3);
    assert(data[newPivotIndex] == 5);
    
    for(int i = 0; i < newPivotIndex; i++) assert(data[i] < 5);
    for(int i = newPivotIndex + 1; i < (int)data.size(); i++) assert(data[i] >= 5);
    
    // Verify metrics triggered (exact numbers depend on swap counts, 
    // but they must be strictly greater than 0)
    assert(metrics.comparisons > 0);
    assert(metrics.accesses > 0);
    
    std::cout << "testQuickSelectPartition passed.\n";
}

void testMoMMedianOfSmallGroup() {
    MedianOfMediansSelection<int> mom(5);
    MetricsContext metrics;
    std::vector<int> data = {10, 2, 8, 4, 6}; // Unsorted group

    // Median of {2, 4, 6, 8, 10} is 6
    int med = mom.get_median_of_small_group(data, 0, 4, metrics);
    assert(med == 6);
    assert(metrics.comparisons > 0);
    
    std::cout << "testMoMMedianOfSmallGroup passed.\n";
}

void testMoMGetDeterministicPivot() {
    // 15 elements. Groups of 5.
    // Group 1: 12, 15, 11, 2, 9  -> sorted: 2, 9, 11, 12, 15 -> median: 11
    // Group 2: 5, 0, 8, 3, 14    -> sorted: 0, 3, 5, 8, 14   -> median: 5
    // Group 3: 1, 7, 10, 4, 13   -> sorted: 1, 4, 7, 10, 13  -> median: 7
    // Medians array: {11, 5, 7}  -> sorted: 5, 7, 11         -> median of medians: 7
    
    std::vector<int> data = {
        12, 15, 11, 2, 9,
        5, 0, 8, 3, 14,
        1, 7, 10, 4, 13
    };

    MedianOfMediansSelection<int> mom(5);
    MetricsContext metrics;
    
    int pivot = mom.get_pivot(data, 0, data.size() - 1, metrics);
    assert(pivot == 7);
    
    std::cout << "testMoMGetDeterministicPivot passed.\n";
}

void testCliParser() {
    // Input format: n=5 elements, followed by the 5 elements, followed by k=2
    // Elements: 7, 10, 4, 3, 20
    // Sorted: 3, 4, 7, 10, 20
    // k = 2 (0-indexed, so the 3rd smallest element, which is 7)
    std::string inputStr = "5\n7\n10\n4\n3\n20\n2\n";
    
    std::stringstream mockInput(inputStr);
    std::stringstream mockOutput;

    // Use our trusted baseline algorithm for the test
    NaiveSelection<int> algo;
    
    processInput(mockInput, mockOutput, algo);

    assert(mockOutput.str() == "7\n");
    std::cout << "testCliParser passed.\n";
}

int main() {
    std::cout << "--- Running Test Suite ---\n";
    testMetricsContext();

    NaiveSelection<int> naiveAlgo;
    runSelectionTests(naiveAlgo, "NaiveSelection");

    testQuickSelectPartition();
    RandomizedQuickSelect<int> quickSelectAlgo(42);
    runSelectionTests(quickSelectAlgo, "RandomizedQuickSelect");

    testMoMMedianOfSmallGroup();
    testMoMGetDeterministicPivot();
    MedianOfMediansSelection<int> momAlgo(5);
    runSelectionTests(momAlgo, "MedianOfMedians");

    // Phase 4 Test
    testCliParser();

    std::cout << "All tests passed!\n";
    return 0;
}