#include <gtest/gtest.h>
#include "DimacsParser.hpp"
#include <sstream>

// Test 1: Verify correct parsing of a standard DIMACS max-flow string
TEST(DimacsParserTest, ParseValidDimacsFormat) {
    std::string dimacs_input = 
        "c This is a comment\n"
        "c Another comment\n"
        "p max 4 5\n"
        "n 1 s\n"
        "n 4 t\n"
        "a 1 2 20\n"
        "a 1 3 10\n"
        "a 2 3 5\n"
        "a 2 4 10\n"
        "a 3 4 20\n";

    std::istringstream in_stream(dimacs_input);
    FlowNetwork network = DimacsParser::parse_from_stream(in_stream);

    // Verify Network Properties
    EXPECT_EQ(network.get_num_vertices(), 4);
    EXPECT_EQ(network.get_source(), 0); // 1-based '1' becomes 0-based '0'
    EXPECT_EQ(network.get_sink(), 3);   // 1-based '4' becomes 0-based '3'
    EXPECT_EQ(network.get_num_edges(), 5);

    // Verify first edge (1 -> 2 with capacity 20, which is 0 -> 1 in our system)
    ASSERT_GT(network.adj_list[0].size(), 0);
    EXPECT_EQ(network.adj_list[0][0].to, 1);
    EXPECT_EQ(network.adj_list[0][0].capacity, 20);
}

// Test 2: Verify that missing source/sink throws an error
TEST(DimacsParserTest, ThrowsOnMissingSourceOrSink) {
    std::string dimacs_input = 
        "p max 2 1\n"
        "a 1 2 10\n";
    // Missing 'n 1 s' and 'n 2 t'

    std::istringstream in_stream(dimacs_input);
    EXPECT_THROW(DimacsParser::parse_from_stream(in_stream), std::runtime_error);
}

// Test 3: Verify that missing 'p' line throws an error
TEST(DimacsParserTest, ThrowsOnMissingPLine) {
    std::string dimacs_input = 
        "n 1 s\n"
        "n 2 t\n"
        "a 1 2 10\n";

    std::istringstream in_stream(dimacs_input);
    EXPECT_THROW(DimacsParser::parse_from_stream(in_stream), std::runtime_error);
}