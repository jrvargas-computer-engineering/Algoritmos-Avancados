# Compiler and Flags
CXX = g++
# -Iinclude tells GCC to look for your .hpp file in the include folder
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -g -fsanitize=address -Iinclude

# Targets
APP_BIN = dijkstra
TEST_BIN = run_tests

# Phony targets (commands that aren't actual files)
.PHONY: all test clean

all: $(APP_BIN)

# Build the main Dijkstra application
$(APP_BIN): src/main.cpp include/KaryMinHeap.hpp
	$(CXX) $(CXXFLAGS) src/main.cpp -o $(APP_BIN)

# Build and run the Test Suite
test: tests/test_heap.cpp include/KaryMinHeap.hpp
	$(CXX) $(CXXFLAGS) tests/test_heap.cpp -o $(TEST_BIN)
	@echo "\n--- Running Unit Tests ---"
	./$(TEST_BIN)

clean:
	rm -f $(APP_BIN) $(TEST_BIN)