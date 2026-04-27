cmake_minimum_required(VERSION 3.14)
project(MaxFlowAssignment)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable CTest (This is what makes the 'make test' command work)
enable_testing()

# Fetch GoogleTest directly from GitHub
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows compatibility
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Tell CMake where to find your .hpp files
include_directories(src/include)

# ==========================================
# 1. Build your core algorithms into a library
# ==========================================
add_library(FlowCore 
    src/FlowNetwork.cpp
    src/EdmondsKarp.cpp
    src/FordFulkersonDFS.cpp
    src/FordFulkersonRandDFS.cpp
    src/FattestPath.cpp       
    src/CapacityScaling.cpp 
    src/Dinitz.cpp            # <-- Fixed: Dinitz was missing!
    src/DimacsParser.cpp
)

# ==========================================
# 2. Build the test executable
# ==========================================
add_executable(FlowTests 
    tests/FlowNetwork_test.cpp
    tests/MetricsReport_test.cpp
    tests/MaxFlowSolvers_test.cpp
    tests/DimacsParser_test.cpp
    tests/IndexedMaxHeap_test.cpp
)

# 3. Link the core library and Google Test to the test executable
target_link_libraries(FlowTests GTest::gtest_main FlowCore)

# 4. Register the tests with CTest
add_test(NAME FlowTestSuite COMMAND FlowTests)


# ==========================================
# 5. Build the main experiment executable
# ==========================================
add_executable(FlowExperiment main.cpp)

# Link the core algorithms to the executable
target_link_libraries(FlowExperiment FlowCore)


# ==========================================
# 6. Custom Command: "make experiments"
# ==========================================

# 6a. Define the generator executable, but EXCLUDE it from standard 'make'
add_executable(washington EXCLUDE_FROM_ALL experiments/new_washington.c)

# Suppress the legacy C warnings
target_compile_options(washington PRIVATE -w)

# 6b. Create the custom "make experiments" command
add_custom_target(experiments
    # Ensure the main C++ program and the generator are compiled first
    DEPENDS washington FlowExperiment 
    
    # Automatically copy the Perl script to the build folder when typed
    COMMAND ${CMAKE_COMMAND} -E copy 
        ${CMAKE_SOURCE_DIR}/experiments/run_experiments.pl 
        ${CMAKE_BINARY_DIR}/run_experiments.pl
        
    COMMENT "Building washington generator and preparing experiments..."
)