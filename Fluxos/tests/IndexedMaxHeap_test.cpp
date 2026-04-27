#include <gtest/gtest.h>
#include "IndexedMaxHeap.hpp"

TEST(IndexedMaxHeapTest, InsertAndExtractMax) {
    IndexedMaxHeap heap(5); // Heap for nodes 0 through 4
    
    heap.insert(0, 10);
    heap.insert(1, 50);
    heap.insert(2, 20);

    // Max should be node 1 (key 50)
    EXPECT_EQ(heap.extract_max(), 1);
    
    // Next max should be node 2 (key 20)
    EXPECT_EQ(heap.extract_max(), 2);
    
    // Last is node 0 (key 10)
    EXPECT_EQ(heap.extract_max(), 0);
    
    EXPECT_TRUE(heap.is_empty());
}

TEST(IndexedMaxHeapTest, UpdateKeyIncreasesPriority) {
    IndexedMaxHeap heap(3);
    heap.insert(0, 10);
    heap.insert(1, 20);
    heap.insert(2, 5);

    // Node 2 suddenly gets a massive bottleneck path
    heap.update_key(2, 100);

    // Now node 2 should be extracted first
    EXPECT_EQ(heap.extract_max(), 2);
    EXPECT_EQ(heap.extract_max(), 1);
}

TEST(IndexedMaxHeapTest, ContainsAndExceptionHandling) {
    IndexedMaxHeap heap(2);
    heap.insert(0, 10);

    EXPECT_TRUE(heap.contains(0));
    EXPECT_FALSE(heap.contains(1));

    // Inserting a node that already exists should throw
    EXPECT_THROW(heap.insert(0, 15), std::invalid_argument);
    
    // Updating a node that isn't in the heap should throw
    EXPECT_THROW(heap.update_key(1, 5), std::invalid_argument);
}