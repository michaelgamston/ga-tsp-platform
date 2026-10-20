#include <iostream>
#include <sstream>
#include <stdexcept>
#include <random> 
#include "tsp_engine/map_io.h"

int main() {

//random number generator 
    unsigned seed = 42;

    try {
        auto dist = parse_dist_matrix(std::cin);
        // ... run GA
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;   // non-zero exit code = failure
    }
}
