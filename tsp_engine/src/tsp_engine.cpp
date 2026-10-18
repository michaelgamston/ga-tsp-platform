#include <iostream>
#include <vector>
#include <random> 
#include <numeric>
#include <algorithm>
#include <string>
#include <sstream>
#include <stdexcept>
#include "tsp_engine/map_io.h"
#include "tsp_engine/individual.h"


class Genetic_algorithm{

    private: 
        //random number generator 
        std::mt19937 m_gen{std::random_device{}()};

        const int m_P; //population size
        const int m_N; //gene size 
        const int m_SEEDS; //number of start solutions 
        const int m_R; // internal interations 
        const float m_M; // mutation rate  

        std::vector<int> m_average; //for storing results at the end of cycle 
        std::vector<int> m_best; // for storung the best result
        std::vector<Individual> m_population; // for storing the population of individuals
        std::vector<std::vector<int>> m_dist_matrix; // matrix storing distances

        //storing best and worst scores
        // int tempbest = 0;
        // int tempworst = 0;



    public: 

        Genetic_algorithm(int p, int n, int seeds, int r, float m) 
        : m_P(p), m_N(n), m_SEEDS(seeds), m_R(r), m_M(m) {
            
            //reserve space in the vector 
            m_population.reserve(m_P);
            for (int i = 0; i < m_P; i++){
                //in each empty space run the Individual constructor
                m_population.emplace_back(m_gen,m_dist_matrix );
            }

        }

};

int main() {
    try {
        auto dist = parse_dist_matrix(std::cin);
        // ... run GA
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;   // non-zero exit code = failure
    }
}
