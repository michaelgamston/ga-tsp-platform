#include <iostream>
#include <sstream>
#include <stdexcept>
#include <random> 
#include <string>
#include "tsp_engine/map_io.h"
#include "tsp_engine/genetic_algorithm.h"

int main(int argc, char* argv[]) {

    //fill out arguments with defaults
    int population = 100;
    int iterations = 100;
    float mutation_rate = 0.1;
    unsigned seed = 42;

    //find any given arugments 
    //this is will only be called by internal logic

    try {

        for(int i = 1; i < argc; i++){

            std::string flag = argv[i];

            if (flag == "--pop") population = std::stoi(argv[++i]);
            else if (flag == "--iter") iterations = std::stoi(argv[++i]);
            else if (flag == "--mut_rate") mutation_rate = std::stof(argv[++i]);
            else if (flag == "--seed") seed = std::stoul(argv[++i]);
        }
    
        Genetic_algorithm ga(population, iterations, mutation_rate, std::cin, seed);
        ga.run_ga(std::cout);

    } catch (const std::exception& e) {

        std::cerr << "Error: " << e.what() << std::endl;
        return 1;   // non-zero exit code = failure

    }
}
