CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc/include
BUILD_DIR = build
BENCH_DIR = build/bench

# =============================================================================
# BINARY TARGET REGISTRY
# =============================================================================
EXEC          = $(BUILD_DIR)/matcher
TEST_EXEC     = $(BUILD_DIR)/test_runner
TEST_MATCHERS = $(BUILD_DIR)/matchers_test_runner
TEST_HUNGARIAN = $(BUILD_DIR)/hungarian_test_runner
TEST_JOHNSON  = $(BUILD_DIR)/johnson_test_runner
BENCHMARK_EXEC = $(BUILD_DIR)/benchmark_runner
TEST_ORACLES  = $(BUILD_DIR)/oracle_test_runner

# =============================================================================
# OBJECT COMPONENT CLUSTERS
# Production and test objects use BUILD_DIR (no -DBENCHMARK_MODE).
# Benchmark objects use BENCH_DIR (compiled with -DBENCHMARK_MODE).
# =============================================================================
PROD_OBJS = $(BUILD_DIR)/BipartiteGraph.o \
            $(BUILD_DIR)/GraphGenerator.o \
            $(BUILD_DIR)/BellmanFordStrategy.o \
            $(BUILD_DIR)/JohnsonDijkstraStrategy.o \
            $(BUILD_DIR)/HungarianMatcher.o \
            $(BUILD_DIR)/main.o

STEP1_OBJS = $(BUILD_DIR)/BipartiteGraph.o \
             $(BUILD_DIR)/GraphGenerator.o \
             $(BUILD_DIR)/test_BipartiteGraph.o \
             $(BUILD_DIR)/test_GraphGenerator.o \
             $(BUILD_DIR)/test_IOHandler.o

STEP2_OBJS = $(BUILD_DIR)/BipartiteGraph.o \
             $(BUILD_DIR)/GraphGenerator.o \
             $(BUILD_DIR)/SimpleAugmentingMatcher.o \
             $(BUILD_DIR)/HopcroftKarpMatcher.o \
             $(BUILD_DIR)/test_Matchers.o

STEP3_OBJS = $(BUILD_DIR)/BipartiteGraph.o \
             $(BUILD_DIR)/GraphGenerator.o \
             $(BUILD_DIR)/BellmanFordStrategy.o \
             $(BUILD_DIR)/HungarianMatcher.o \
             $(BUILD_DIR)/test_HungarianMatcher.o

STEP4_OBJS = $(BUILD_DIR)/BipartiteGraph.o \
             $(BUILD_DIR)/GraphGenerator.o \
             $(BUILD_DIR)/JohnsonDijkstraStrategy.o \
             $(BUILD_DIR)/HungarianMatcher.o \
             $(BUILD_DIR)/test_JohnsonDijkstra.o

BENCHMARK_OBJS = $(BENCH_DIR)/BipartiteGraph.o \
                 $(BENCH_DIR)/GraphGenerator.o \
                 $(BENCH_DIR)/Oracle.o \
                 $(BENCH_DIR)/SimpleAugmentingMatcher.o \
                 $(BENCH_DIR)/HopcroftKarpMatcher.o \
                 $(BENCH_DIR)/BellmanFordStrategy.o \
                 $(BENCH_DIR)/JohnsonDijkstraStrategy.o \
                 $(BENCH_DIR)/HungarianMatcher.o \
                 $(BENCH_DIR)/benchmark_main.o

ORACLE_OBJS = $(BUILD_DIR)/BipartiteGraph.o \
              $(BUILD_DIR)/GraphGenerator.o \
              $(BUILD_DIR)/Oracle.o \
              $(BUILD_DIR)/SimpleAugmentingMatcher.o \
              $(BUILD_DIR)/HopcroftKarpMatcher.o \
              $(BUILD_DIR)/BellmanFordStrategy.o \
              $(BUILD_DIR)/JohnsonDijkstraStrategy.o \
              $(BUILD_DIR)/HungarianMatcher.o \
              $(BUILD_DIR)/test_Group0_Oracles.o

# =============================================================================
# TOP-LEVEL ORCHESTRATION TARGETS
# =============================================================================
.PHONY: all test benchmark_runner analyze clean scaffold

all: scaffold $(EXEC)

# Runs the Python analysis over already-generated data in data/combined/,
# writing tables and figures to analysis_output/. Use after a run with
# './run_experiments.sh ... --no-analysis'.
analyze:
	python3 -m analysis.run_all --data-dir data/combined --out-dir analysis_output

scaffold:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BENCH_DIR)
	@mkdir -p raw
	@mkdir -p data/combined
	@mkdir -p analysis_output
	@mkdir -p figures

test: scaffold $(TEST_EXEC) $(TEST_MATCHERS) $(TEST_HUNGARIAN) $(TEST_JOHNSON)
	@echo "\n>>> EXECUTING STEP 1: MEMORY & I/O BASELINES <<<"
	@./$(TEST_EXEC)
	@echo "\n>>> EXECUTING STEP 2: UNWEIGHTED MATCHING BASELINES <<<"
	@./$(TEST_MATCHERS)
	@echo "\n>>> EXECUTING STEP 3: HUNGARIAN BASELINE PROOF <<<"
	@./$(TEST_HUNGARIAN)
	@echo "\n>>> EXECUTING STEP 4: JOHNSON-DIJKSTRA POTENTIAL PROOF <<<"
	@./$(TEST_JOHNSON)

benchmark_runner: scaffold $(BENCHMARK_EXEC)

# =============================================================================
# EXECUTABLE LINKING RECIPES
# =============================================================================
$(EXEC): $(PROD_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(PROD_OBJS)

$(TEST_EXEC): $(STEP1_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(STEP1_OBJS)

$(TEST_MATCHERS): $(STEP2_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(STEP2_OBJS)

$(TEST_HUNGARIAN): $(STEP3_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(STEP3_OBJS)

$(TEST_JOHNSON): $(STEP4_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(STEP4_OBJS)

$(BENCHMARK_EXEC): $(BENCHMARK_OBJS)
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -o $@ $(BENCHMARK_OBJS)

$(TEST_ORACLES): $(ORACLE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(ORACLE_OBJS)

# =============================================================================
# PRODUCTION COMPILATION UNITS  (no -DBENCHMARK_MODE)
# =============================================================================
$(BUILD_DIR)/BipartiteGraph.o: src/BipartiteGraph.cpp src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -c src/BipartiteGraph.cpp -o $@

$(BUILD_DIR)/GraphGenerator.o: src/GraphGenerator.cpp src/include/GraphGenerator.h src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -c src/GraphGenerator.cpp -o $@

$(BUILD_DIR)/main.o: src/main.cpp src/include/BipartiteGraph.h src/include/IOHandler.h src/include/HungarianMatcher.h src/include/JohnsonDijkstraStrategy.h
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o $@

$(BUILD_DIR)/SimpleAugmentingMatcher.o: src/SimpleAugmentingMatcher.cpp src/include/SimpleAugmentingMatcher.h src/include/IMatcher.h
	$(CXX) $(CXXFLAGS) -c src/SimpleAugmentingMatcher.cpp -o $@

$(BUILD_DIR)/HopcroftKarpMatcher.o: src/HopcroftKarpMatcher.cpp src/include/HopcroftKarpMatcher.h src/include/IMatcher.h
	$(CXX) $(CXXFLAGS) -c src/HopcroftKarpMatcher.cpp -o $@

$(BUILD_DIR)/BellmanFordStrategy.o: src/BellmanFordStrategy.cpp src/include/BellmanFordStrategy.h src/include/IPathfindingStrategy.h
	$(CXX) $(CXXFLAGS) -c src/BellmanFordStrategy.cpp -o $@

$(BUILD_DIR)/JohnsonDijkstraStrategy.o: src/JohnsonDijkstraStrategy.cpp src/include/JohnsonDijkstraStrategy.h src/include/IPathfindingStrategy.h
	$(CXX) $(CXXFLAGS) -c src/JohnsonDijkstraStrategy.cpp -o $@

$(BUILD_DIR)/HungarianMatcher.o: src/HungarianMatcher.cpp src/include/HungarianMatcher.h src/include/IMatcher.h src/include/IPathfindingStrategy.h
	$(CXX) $(CXXFLAGS) -c src/HungarianMatcher.cpp -o $@

$(BUILD_DIR)/Oracle.o: src/Oracle.cpp src/include/Oracle.h src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -c src/Oracle.cpp -o $@

$(BUILD_DIR)/test_BipartiteGraph.o: src/tests/test_BipartiteGraph.cpp src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -c src/tests/test_BipartiteGraph.cpp -o $@

$(BUILD_DIR)/test_GraphGenerator.o: src/tests/test_GraphGenerator.cpp src/include/GraphGenerator.h src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -c src/tests/test_GraphGenerator.cpp -o $@

$(BUILD_DIR)/test_IOHandler.o: src/tests/test_IOHandler.cpp src/include/IOHandler.h src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -c src/tests/test_IOHandler.cpp -o $@

$(BUILD_DIR)/test_Matchers.o: src/tests/test_Matchers.cpp src/include/SimpleAugmentingMatcher.h src/include/HopcroftKarpMatcher.h
	$(CXX) $(CXXFLAGS) -c src/tests/test_Matchers.cpp -o $@

$(BUILD_DIR)/test_HungarianMatcher.o: src/tests/test_HungarianMatcher.cpp src/include/HungarianMatcher.h src/include/BellmanFordStrategy.h src/include/BipartiteGraph.h src/include/GraphGenerator.h
	$(CXX) $(CXXFLAGS) -c src/tests/test_HungarianMatcher.cpp -o $@

$(BUILD_DIR)/test_JohnsonDijkstra.o: src/tests/test_JohnsonDijkstra.cpp src/include/JohnsonDijkstraStrategy.h src/include/HungarianMatcher.h src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -c src/tests/test_JohnsonDijkstra.cpp -o $@

$(BUILD_DIR)/test_Group0_Oracles.o: src/tests/test_Group0_Oracles.cpp src/include/Oracle.h src/include/SimpleAugmentingMatcher.h src/include/HopcroftKarpMatcher.h src/include/BellmanFordStrategy.h src/include/JohnsonDijkstraStrategy.h src/include/HungarianMatcher.h src/include/BipartiteGraph.h src/include/GraphGenerator.h
	$(CXX) $(CXXFLAGS) -c src/tests/test_Group0_Oracles.cpp -o $@

# =============================================================================
# BENCHMARK COMPILATION UNITS  (compiled with -DBENCHMARK_MODE)
# =============================================================================
$(BENCH_DIR)/BipartiteGraph.o: src/BipartiteGraph.cpp src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/BipartiteGraph.cpp -o $@

$(BENCH_DIR)/GraphGenerator.o: src/GraphGenerator.cpp src/include/GraphGenerator.h src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/GraphGenerator.cpp -o $@

$(BENCH_DIR)/Oracle.o: src/Oracle.cpp src/include/Oracle.h src/include/BipartiteGraph.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/Oracle.cpp -o $@

$(BENCH_DIR)/SimpleAugmentingMatcher.o: src/SimpleAugmentingMatcher.cpp src/include/SimpleAugmentingMatcher.h src/include/IMatcher.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/SimpleAugmentingMatcher.cpp -o $@

$(BENCH_DIR)/HopcroftKarpMatcher.o: src/HopcroftKarpMatcher.cpp src/include/HopcroftKarpMatcher.h src/include/IMatcher.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/HopcroftKarpMatcher.cpp -o $@

$(BENCH_DIR)/BellmanFordStrategy.o: src/BellmanFordStrategy.cpp src/include/BellmanFordStrategy.h src/include/IPathfindingStrategy.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/BellmanFordStrategy.cpp -o $@

$(BENCH_DIR)/JohnsonDijkstraStrategy.o: src/JohnsonDijkstraStrategy.cpp src/include/JohnsonDijkstraStrategy.h src/include/IPathfindingStrategy.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/JohnsonDijkstraStrategy.cpp -o $@

$(BENCH_DIR)/HungarianMatcher.o: src/HungarianMatcher.cpp src/include/HungarianMatcher.h src/include/IMatcher.h src/include/IPathfindingStrategy.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/HungarianMatcher.cpp -o $@

$(BENCH_DIR)/benchmark_main.o: src/benchmark_main.cpp src/include/CSVLogger.h
	$(CXX) $(CXXFLAGS) -DBENCHMARK_MODE -c src/benchmark_main.cpp -o $@

# =============================================================================
# CLEANUP
# =============================================================================
# Removes only compiled objects and binaries. Experiment data (raw/, data/,
# analysis_output/, figures/) is never deleted automatically.
clean:
	rm -rf $(BUILD_DIR)
