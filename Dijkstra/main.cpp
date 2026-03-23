#include <iostream>
#include <string>
#include <limits>
#include <chrono>
#include <fstream> 
#include "SparseGraph.hpp"
#include "Dijkstra.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        return 1; 
    }
    
    int k = 4; 
    if (argc >= 4) {
        k = std::stoi(argv[3]);
    }

    int source = std::stoi(argv[1]) - 1;
    int dest = std::stoi(argv[2]) - 1;

    try {
        /* =================*/
        /* MEASURE I/O TIME */
        /* =================*/
        auto start_io = std::chrono::high_resolution_clock::now();
        SparseGraph graph = SparseGraph::fromDIMACS();
        auto end_io = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> io_ms = end_io - start_io;

        int numVertices = graph.getNumVertices();
       
        /* =======================*/
        /* MEASURE ALGORITHM TIME */
        /* =======================*/
        auto start_math = std::chrono::high_resolution_clock::now();
        Dijkstra solver;
        solver.run(source, dest, graph, k); // 4-ary heap
        auto end_math = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> math_ms = end_math - start_math;


        /* ==========================*/
        /* STRICT STDOUT REQUIREMENT */
        /* ==========================*/
        int dist = solver.getDistance(dest);
        const int INF = std::numeric_limits<int>::max();

        if (dist == INF) {
            std::cout << "inf\n";
        } else {
            std::cout << dist << "\n";
        }

        /* ===============*/
        /* FETCH COUNTERS */
        /* ===============*/
        long long inserts = solver.getInsertCounter();
        long long deletemins = solver.getDeleteMinCounter();
        long long updates = solver.getUpdateCounter();
        long long siftUps = solver.getSiftUps();
        long long siftDowns = solver.getSiftDowns();
      
        /* ============*/
        /* REPORT FILE */
        /* ============*/
        std::ofstream reportFile("performance_metrics.csv", std::ios::app);
        if (reportFile.is_open()) {
            reportFile.seekp(0, std::ios::end);
            if (reportFile.tellp() == 0) {
                reportFile << "Vertices,Source,Dest,IO_Time,Algo_Time,I,D,U,Sift_Up,Sift_Down\n";
            }

            reportFile << numVertices << "," 
                       << (source + 1) << "," 
                       << (dest + 1) << "," 
                       << io_ms.count() << "," 
                       << math_ms.count() << ","
                       << inserts << ","
                       << deletemins << ","
                       << updates << ","
                       << siftUps << ","       
                       << siftDowns << "\n";   
            reportFile.close();
        }
    } catch (...) {
        return 1;
    }

    return 0;
}