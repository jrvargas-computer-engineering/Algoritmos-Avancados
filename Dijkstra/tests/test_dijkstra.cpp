#include <iostream>
#include <vector>
#include <string>
#include "../include/SparseGraph.hpp"
#include "../include/Dijkstra.hpp"

// --- TEST UTILITIES ---
void assert_eq(int actual, int expected, const std::string& msg) {
    if (actual != expected) {
        std::cerr << "❌ TEST FAILED: " << msg << "\n";
        std::cerr << "   Expected: " << expected << " | Actual: " << actual << "\n";
        exit(1);
    }
}

// --- TEST CASES ---

void test_straight_line() {
    SparseGraph g(4);
    // 0 -> 1 -> 2 -> 3
    g.addEdge(0, 1, 5);
    g.addEdge(1, 2, 5);
    g.addEdge(2, 3, 5);

    Dijkstra solver;
    solver.run(0, g, 2); // Binary heap

    assert_eq(solver.getDistance(3), 15, "Straight line distance to node 3");
    
    std::vector<int> expected_path = {0, 1, 2, 3};
    std::vector<int> actual_path = solver.getPath(3);
    assert_eq(actual_path.size(), 4, "Straight line path length");
    for (size_t i = 0; i < expected_path.size(); ++i) {
        assert_eq(actual_path[i], expected_path[i], "Straight line path node " + std::to_string(i));
    }
    
    std::cout << "✅ Passed: Straight Line Graph\n";
}

void test_diamond_graph() {
    SparseGraph g(4);
    // The "Diamond": Two ways to get from 0 to 3
    // Path A: 0 -> 1 -> 3 (Weight: 2 + 10 = 12)
    // Path B: 0 -> 2 -> 3 (Weight: 5 + 1 = 6) <-- Shortest!
    g.addEdge(0, 1, 2);
    g.addEdge(0, 2, 5);
    g.addEdge(1, 3, 10);
    g.addEdge(2, 3, 1);

    Dijkstra solver;
    solver.run(0, g, 3); // Ternary heap

    assert_eq(solver.getDistance(3), 6, "Diamond graph shortest distance");
    
    std::vector<int> path = solver.getPath(3);
    assert_eq(path[1], 2, "Algorithm should choose node 2 over node 1");
    
    std::cout << "✅ Passed: Diamond Graph (Split Path)\n";
}

void test_unreachable_node() {
    SparseGraph g(5);
    // 0 -> 1 -> 2. Nodes 3 and 4 are completely disconnected.
    g.addEdge(0, 1, 10);
    g.addEdge(1, 2, 10);

    Dijkstra solver;
    solver.run(0, g, 4); // 4-ary heap

    int INF = std::numeric_limits<int>::max();
    assert_eq(solver.getDistance(3), INF, "Distance to unreachable node should be INF");
    assert_eq(solver.getPath(3).size(), 0, "Path to unreachable node should be empty");
    
    std::cout << "✅ Passed: Unreachable Nodes\n";
}

void test_file_to_dijkstra_pipeline() {
    std::cout << "Running DIMACS Pipeline Test...\n";
    std::string filename = "temp_pipeline_dimacs.gr";
    
    // 1. Create a temporary file representing a complex graph in DIMACS format
    // DIMACS is 1-indexed. We are building the exact same graph as before.
    std::ofstream outfile(filename);
    outfile << "c Integration test graph\n";
    outfile << "p sp 5 6\n";   // 5 Vertices, 6 Edges
    outfile << "a 1 2 2\n";    // 1 -> 2 (w: 2)  [Internal: 0 -> 1]
    outfile << "a 1 3 4\n";    // 1 -> 3 (w: 4)  [Internal: 0 -> 2]
    outfile << "a 2 3 1\n";    // 2 -> 3 (w: 1)  [Internal: 1 -> 2] <-- Makes 1->2->3 faster than 1->3
    outfile << "a 2 4 7\n";    // 2 -> 4 (w: 7)  [Internal: 1 -> 3]
    outfile << "a 3 5 3\n";    // 3 -> 5 (w: 3)  [Internal: 2 -> 4]
    outfile << "a 5 4 2\n";    // 5 -> 4 (w: 2)  [Internal: 4 -> 3] <-- Makes 1->2->3->5->4 faster
    outfile.close();

    // 2. Redirect std::cin to read from our file
    std::ifstream infile(filename);
    std::streambuf* originalCinBuffer = std::cin.rdbuf();
    std::cin.rdbuf(infile.rdbuf());

    try {
        // 3. Load the graph from stdin
        SparseGraph g = SparseGraph::fromDIMACS();
        
        // Restore std::cin as soon as we finish reading
        std::cin.rdbuf(originalCinBuffer);

        // 4. Run Dijkstra starting at internal node 0 (which is DIMACS node 1)
        Dijkstra solver;
        solver.run(0, g, 3); // 3-ary heap

        // 5. Verify the math using our 0-indexed internal array
        // Internal Node 1 (DIMACS 2)
        assert_eq(solver.getDistance(1), 2, "Pipeline: Distance to internal node 1");
        // Internal Node 2 (DIMACS 3)
        assert_eq(solver.getDistance(2), 3, "Pipeline: Distance to internal node 2 (via 1)");
        // Internal Node 4 (DIMACS 5)
        assert_eq(solver.getDistance(4), 6, "Pipeline: Distance to internal node 4 (via 1, 2)");
        // Internal Node 3 (DIMACS 4)
        assert_eq(solver.getDistance(3), 8, "Pipeline: Distance to internal node 3 (via 1, 2, 4)");

        std::cout << "✅ Passed: End-to-End DIMACS Pipeline\n";
    } catch (const std::exception& e) {
        // Guarantee safety on crash
        std::cin.rdbuf(originalCinBuffer);
        std::cerr << "❌ Pipeline Failed: " << e.what() << "\n";
        std::remove(filename.c_str());
        exit(1);
    }

    // 6. Clean up the temporary file
    std::remove(filename.c_str());
}

// --- MAIN RUNNER ---
int main() {
    std::cout << "--- DIJKSTRA INTEGRATION TESTS ---\n";
    try {
        test_straight_line();
        test_diamond_graph();
        test_unreachable_node();
        test_file_to_dijkstra_pipeline();
        std::cout << "\n🎉 ALL DIJKSTRA TESTS PASSED!\n";
    } catch (const std::exception& e) {
        std::cerr << "💥 Runtime Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}