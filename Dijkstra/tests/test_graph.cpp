#include "../include/SparseGraph.hpp"
#include <iostream>
#include <cassert>
#include <vector>

void test_manual_insertion() {
    std::cout << "Running Manual Insertion Test..." << std::endl;
    SparseGraph g(3); // 3 vertices: 0, 1, 2
    
    g.addEdge(0, 1, 10);
    g.addEdge(0, 2, 5);
    g.addEdge(1, 2, 2);

    auto neighbors0 = g.getNeighbors(0);
    // There should be exactly 2 neighbors for vertex 0
    assert(neighbors0.size() == 2);

    bool found_1 = false;
    for (const auto& edge : neighbors0) {
        if (edge.to == 1) {
            assert(edge.weight == 10);
            found_1 = true;
        }
    }
    assert(found_1 && "Edge 0->1 not found!");
    std::cout << "✅ Manual Insertion Passed!" << std::endl;
}

void test_out_of_bounds() {
    std::cout << "Running Bounds Safety Test..." << std::endl;
    SparseGraph g(2);
    // Should not crash even if we try to add an edge from a non-existent vertex
    g.addEdge(99, 0, 10); 
    assert(g.getNumVertices() == 2);
    std::cout << "✅ Bounds Safety Passed!" << std::endl;
}

void test_file_reading() {
    std::cout << "Running DIMACS File Reading Test..." << std::endl;
    
    // 1. Create a temporary test file in DIMACS format
    std::ofstream outfile("temp_test_dimacs.gr");
    outfile << "c This is a test comment\n";
    outfile << "p sp 3 2\n";   // 3 vertices, 2 edges
    outfile << "a 1 2 50\n";   // 1 -> 2 (weight 50)
    outfile << "a 2 3 25\n";   // 2 -> 3 (weight 25)
    outfile.close();

    // 2. Redirect std::cin to read from our file instead of the keyboard
    std::ifstream infile("temp_test_dimacs.gr");
    std::streambuf* originalCinBuffer = std::cin.rdbuf(); // Save the original buffer
    std::cin.rdbuf(infile.rdbuf());                       // Swap it to the file

    try {
        // 3. Read using the new DIMACS method
        SparseGraph g = SparseGraph::fromDIMACS();
        
        // Restore std::cin immediately after we are done reading
        std::cin.rdbuf(originalCinBuffer);

        assert(g.getNumVertices() == 3);
        
        // 4. Verify the 1-indexed to 0-indexed conversion worked!
        // DIMACS Node '2' is stored at internal index '1'
        // DIMACS Node '3' is stored at internal index '2'
        auto neighbors = g.getNeighbors(1); 
        
        assert(neighbors.front().to == 2);
        assert(neighbors.front().weight == 25);
        
        std::cout << "✅ DIMACS File Reading Passed!" << std::endl;
    } catch (const std::exception& e) {
        // Ensure std::cin is restored even if the test crashes
        std::cin.rdbuf(originalCinBuffer);
        std::cerr << "❌ DIMACS File Reading Failed: " << e.what() << std::endl;
        exit(1);
    }
    
    // 5. Cleanup
    std::remove("temp_test_dimacs.gr");
}

int main() {
    std::cout << "--- SPARSE GRAPH UNIT TESTS ---" << std::endl;
    test_manual_insertion();
    test_out_of_bounds();
    test_file_reading();
    std::cout << "\n🎉 ALL GRAPH TESTS PASSED!" << std::endl;
    return 0;
}

