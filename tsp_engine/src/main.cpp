#include <iostream>
#include <sstream>
#include <stdexcept>
#include "tsp_engine/map_io.h"

int main() {
    try {
        auto dist = parse_dist_matrix(std::cin);
        // ... run GA
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;   // non-zero exit code = failure
    }
}
