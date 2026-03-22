#include "../include/KaryMinHeap.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <random>

// --- DIAGNOSTIC HELPERS ---
void assert_eq(int actual, int expected, const std::string& msg) {
    if (actual != expected) {
        std::cerr << "❌ TEST FAILED: " << msg << std::endl;
        std::cerr << "   Expected: " << expected << " | Actual: " << actual << std::endl;
        exit(1); 
    }
    std::cout << "✅ Passed: " << msg << std::endl;
}

// --- 1. SORTING ORDER TEST ---
void test_sorting_order() {
    KaryMinHeap heap(3); // Ternary heap
    std::vector<int> values = {15, 10, 3, 8, 1};
    for(int v : values) heap.push(v, 0);

    assert_eq(heap.pop().distance, 1,  "Min element (1)");
    assert_eq(heap.pop().distance, 3,  "Second element (3)");
    assert_eq(heap.pop().distance, 8,  "Third element (8)");
    assert_eq(heap.pop().distance, 10, "Fourth element (10)");
    assert_eq(heap.pop().distance, 15, "Fifth element (15)");
}

void test_duplicate_values() {
    int k = 3;
    KaryMinHeap heap(k);
    
    // Insert several nodes with the SAME distance but different vertex IDs
    heap.push(10, 101);
    heap.push(10, 102);
    heap.push(10, 103);
    heap.push(5,  500); // A smaller one
    heap.push(20, 200); // A larger one

    // 1. First one out must be 5
    assert_eq(heap.pop().distance, 5, "Min element (5) before duplicates");

    // 2. Next three MUST be 10, regardless of vertex ID order
    assert_eq(heap.pop().distance, 10, "First duplicate (10)");
    assert_eq(heap.pop().distance, 10, "Second duplicate (10)");
    assert_eq(heap.pop().distance, 10, "Third duplicate (10)");

    // 3. Last one must be 20
    assert_eq(heap.pop().distance, 20, "Last element (20) after duplicates");
    
    std::cout << "✅ Passed: Duplicate Values handling" << std::endl;
}

// --- 2. BRANCHING FACTOR TEST ---
void test_branching_factor() {
    // Testing a very wide heap (k=10) to ensure the math for many children is correct
    KaryMinHeap heap(10); 
    for(int i = 50; i >= 1; --i) heap.push(i, i);
    
    assert_eq(heap.pop().distance, 1, "Wide heap (k=10) first pop");
    assert_eq(heap.pop().distance, 2, "Wide heap (k=10) second pop");
}

// --- 3. STRESS TEST ---
void run_stress_test(int k, int numElements) {
    std::cout << "\n🚀 Starting Stress Test (k=" << k << ", N=" << numElements << ")..." << std::endl;
    
    KaryMinHeap heap(k);
    std::vector<int> reference;
    std::mt19937 rng(1337); 
    std::uniform_int_distribution<int> dist(1, 1000000);

    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < numElements; ++i) {
        int val = dist(rng);
        heap.push(val, i);
        reference.push_back(val);
    }

    std::sort(reference.begin(), reference.end());

    for(int i = 0; i < numElements; ++i) {
        if (heap.pop().distance != reference[i]) {
            std::cerr << "❌ Stress Test Failed at index " << i << std::endl;
            exit(1);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "✨ Completed in " << duration.count() << "s" << std::endl;
}

// --- MAIN RUNNER ---
int main() {
    std::cout << "--- HEAP UNIT TESTS ---" << std::endl;
    test_sorting_order();
    test_duplicate_values();
    test_branching_factor();

    std::cout << "\n--- HEAP STRESS TESTS ---" << std::endl;
    run_stress_test(2, 100000);  // Binary
    run_stress_test(4, 100000);  // 4-ary
    run_stress_test(8, 100000);  // 8-ary

    std::cout << "\n🎉 ALL TESTS PASSED!" << std::endl;
    return 0;
}