# Directories
BUILD_DIR = build
SRC_DIR = src
TEST_DIR = test
INC_DIR = include

# Compiler settings
CXX = g++
INCLUDES = -I$(INC_DIR)

# Flags for TDD/Debugging (Catching memory leaks & out-of-bounds)
CXXFLAGS_DEBUG = -std=c++17 $(INCLUDES) -Wall -Wextra -g -fsanitize=address

# Flags for Experiments (Optimized, NO AddressSanitizer for accurate timing)
CXXFLAGS_RELEASE = -std=c++17 $(INCLUDES) -O3 -DNDEBUG

# Executable paths
MAIN_EXEC = $(BUILD_DIR)/main_app
TEST_EXEC = $(BUILD_DIR)/run_tests
EXP_EXEC = $(BUILD_DIR)/run_experiments

.PHONY: all test clean run_experiments

# Default target builds everything
all: $(BUILD_DIR) $(MAIN_EXEC) $(TEST_EXEC) $(EXP_EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 1. Main CLI app
$(MAIN_EXEC): $(SRC_DIR)/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS_DEBUG) $< -o $@

# 2. Test runner
$(TEST_EXEC): $(TEST_DIR)/test_main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS_DEBUG) $< -o $@

# 3. Experiment runner (Uses RELEASE flags for accurate timing)
$(EXP_EXEC): $(SRC_DIR)/ExperimentRunner.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS_RELEASE) $< -o $@

# TDD Command
test: $(TEST_EXEC)
	@echo "--- Executing Tests ---"
	./$(TEST_EXEC)

# Clean command: removes the build directory, leaving generated data (CSV, PNG) intact
clean:
	@echo "--- Cleaning build directory ---"
	rm -rf $(BUILD_DIR)