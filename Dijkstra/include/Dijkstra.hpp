#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <vector>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include "SparseGraph.hpp"
#include "KaryMinHeap.hpp"

class Dijkstra {
private:
    const int INF = std::numeric_limits<int>::max();
    int numVertices = 0; 
    std::vector<int> distances;
    std::vector<int> parents;
    long long insertCounter = 0;
    long long deleteMinCounter = 0; 
    long long updateCounter = 0;
    long long siftUpsCounter = 0;
    long long siftDownsCounter = 0;

public:
    long long getInsertCounter(){return insertCounter;};
    long long getDeleteMinCounter(){return deleteMinCounter;}; 
    long long getUpdateCounter(){return updateCounter;};
    long long getSiftUps() const { return siftUpsCounter; }
    long long getSiftDowns() const { return siftDownsCounter; }

    void run(int startNode, int destNode, const SparseGraph& graph, int k = 4) {              
        numVertices = graph.getNumVertices();

        if (startNode < 0 || startNode >= numVertices) {
            throw std::out_of_range("Start node is out of bounds.");
        }

        // Reset and size our state arrays for the current graph
        distances.assign(numVertices, INF);
        parents.assign(numVertices, -1);
        
        insertCounter = 0;
        deleteMinCounter = 0;
        updateCounter = 0;
        siftUpsCounter = 0;
        siftDownsCounter = 0; 

        KaryMinHeap heap(k);

        distances[startNode] = 0;
        heap.push(0, startNode);
        insertCounter++;

        while (!heap.isEmpty()) {
            Node current = heap.pop();
            int u = current.vertex;
            int d = current.distance;

            // Lazy Deletion
            if (d > distances[u]) {
                continue;
            }
            
            // --- DELETEMIN: Node 'u' is permanently extracted and expanded ---
            deleteMinCounter++;
    
            // early exit
            if (destNode != -1 && u == destNode) {
                break; // We found the target, stop processing the graph!
            }
            //


            
            for (const auto& edge : graph.getNeighbors(u)) {
                int v = edge.to;
                int weight = edge.weight;

                // Relaxation Step
                if (distances[u] + weight < distances[v]) {

                    if(distances[v] == INF){
                        // --- INSERT: if it was INF, this is the first time we discovered
                        insertCounter++; 
                    }else{
                        // --- UPDATE
                        updateCounter++;
                    }

                    distances[v] = distances[u] + weight;
                    parents[v] = u; //path reconstruction
                    heap.push(distances[v], v);
                
                }
            }
        }
        siftUpsCounter = heap.getSiftUpCounting();
        siftDownsCounter = heap.getSiftDownCounting();
    }

    int getDistance(int target) const {
        if (target < 0 || target >= numVertices) {
            return INF;
        }
        return distances[target];
    }

    std::vector<int> getPath(int target) const {
        std::vector<int> path;
        
        if (target < 0 || target >= numVertices || distances[target] == INF) {
            return path; 
        }
        for (int v = target; v != -1; v = parents[v]) {
            path.push_back(v);
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

#endif