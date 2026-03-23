# Compiler and Flags
CXX = g++
# -Iinclude tells GCC to look for your .hpp files in the include folder
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -g -fsanitize=address -Iinclude

# Targets
APP_BIN = dijkstra
GRAPH_TEST_BIN = run_graph_tests
HEAP_TEST_BIN = run_heap_tests
DIJKSTRA_TEST_BIN = run_dijkstra_tests

# Phony targets (commands that aren't actual files)
.PHONY: all test test_heap test_graph test_dijkstra clean

# Default target: builds the main app
all: $(APP_BIN)

# Build the main Dijkstra application
$(APP_BIN): main.cpp include/SparseGraph.hpp include/Dijkstra.hpp include/KaryMinHeap.hpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(APP_BIN)

# Run only Heap tests
test_heap: tests/test_heap.cpp include/KaryMinHeap.hpp
	$(CXX) $(CXXFLAGS) tests/test_heap.cpp -o $(HEAP_TEST_BIN)
	./$(HEAP_TEST_BIN)

# Run only Graph tests
test_graph: tests/test_graph.cpp include/SparseGraph.hpp
	$(CXX) $(CXXFLAGS) tests/test_graph.cpp -o $(GRAPH_TEST_BIN)
	./$(GRAPH_TEST_BIN)

# Run only Dijkstra tests
test_dijkstra: tests/test_dijkstra.cpp include/SparseGraph.hpp include/Dijkstra.hpp include/KaryMinHeap.hpp
	$(CXX) $(CXXFLAGS) tests/test_dijkstra.cpp -o $(DIJKSTRA_TEST_BIN)
	@echo "\n--- Running Dijkstra Tests ---"
	./$(DIJKSTRA_TEST_BIN)
    
# Run everything
test: test_heap test_graph test_dijkstra

clean:
	rm -f $(APP_BIN) $(HEAP_TEST_BIN) $(GRAPH_TEST_BIN) $(DIJKSTRA_TEST_BIN)