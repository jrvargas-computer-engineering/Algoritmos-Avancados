#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>

class IndexedMaxHeap {
private:
    std::vector<int> heap;        // Stores node IDs
    std::vector<long long> keys;  // Stores the priorities (capacities)
    std::vector<int> pos;         // pos[node] = index in the heap array
    int size;

    void swap_nodes(int i, int j) {
        std::swap(heap[i], heap[j]);
        pos[heap[i]] = i;
        pos[heap[j]] = j;
    }

    void sift_up(int i) {
        while (i > 0 && keys[heap[(i - 1) / 2]] < keys[heap[i]]) {
            swap_nodes(i, (i - 1) / 2);
            i = (i - 1) / 2;
        }
    }

    void sift_down(int i) {
        while (2 * i + 1 < size) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int largest = i;

            if (left < size && keys[heap[left]] > keys[heap[largest]]) largest = left;
            if (right < size && keys[heap[right]] > keys[heap[largest]]) largest = right;

            if (largest != i) {
                swap_nodes(i, largest);
                i = largest;
            } else {
                break;
            }
        }
    }

public:
    // Initialize heap for 'n' possible elements
    IndexedMaxHeap(int n) {
        keys.assign(n, -1);
        pos.assign(n, -1);
        size = 0;
    }

    bool is_empty() const { return size == 0; }
    
    bool contains(int node) const { return pos[node] != -1; }

    void insert(int node, long long key) {
        if (contains(node)) throw std::invalid_argument("Node already in heap");
        
        heap.push_back(node);
        keys[node] = key;
        pos[node] = size;
        sift_up(size);
        size++;
    }

    void update_key(int node, long long new_key) {
        if (!contains(node)) throw std::invalid_argument("Node not in heap");
        
        long long old_key = keys[node];
        keys[node] = new_key;
        
        int i = pos[node];
        if (new_key > old_key) sift_up(i);
        else sift_down(i);
    }

    int extract_max() {
        if (is_empty()) throw std::out_of_range("Heap is empty");
        
        int max_node = heap[0];
        swap_nodes(0, size - 1);
        
        pos[max_node] = -1; // Mark as removed
        heap.pop_back();
        size--;
        
        if (size > 0) sift_down(0);
        
        return max_node;
    }
};