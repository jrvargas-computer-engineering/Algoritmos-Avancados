#pragma once
#include "FlowNetwork.hpp"
#include <iostream>
#include <string>

class DimacsParser {
public:
    // Reads from any standard input stream and constructs the FlowNetwork
    static FlowNetwork parse_from_stream(std::istream& in);
};