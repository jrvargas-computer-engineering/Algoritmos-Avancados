#include <iostream>
#include <string>
#include <memory>
#include "CliHandler.h"
#include "RandomizedQuickSelect.h"
#include "MedianOfMedians.h"
#include "NaiveSelection.h"

int main(int argc, char* argv[]) {
    // Fast I/O for performance in competitive programming / automated testing
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // Default algorithm
    std::string algo_choice = "quickselect"; 
    
    // Override if provided via command line
    if (argc > 1) {
        algo_choice = argv[1];
    }

    // Use a smart pointer to hold the polymorphic base class
    std::unique_ptr<SelectionAlgorithm<int>> algo;

    if (algo_choice == "naive") {
        algo = std::make_unique<NaiveSelection<int>>();
    } 
    else if (algo_choice == "mom") {
        // Optional: Allow passing group size as a second argument (default to 5)
        int g = (argc > 2) ? std::stoi(argv[2]) : 5;
        algo = std::make_unique<MedianOfMediansSelection<int>>(g);
    } 
    else {
        // Fallback / default to randomized quickselect
        algo = std::make_unique<RandomizedQuickSelect<int>>();
    }

    // Execute the chosen algorithm
    processInput(std::cin, std::cout, *algo);

    return 0;
}