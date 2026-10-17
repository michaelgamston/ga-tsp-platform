#include <iostream>
#include <vector>
#include <random> 
#include <numeric>
#include <algorithm>
#include <string>
#include <sstream>
#include <stdexcept>
#include "tsp_engine/map_io.h"



class Individual{
    public:

        static const int m_PENALTY = 1000000;

        std::vector<int> m_genome; 
        int m_fitness; 

        //using the intialiser list to construct the variables with elements
        //rather than contructing them emmpty and then assigning them
        Individual(int n, std::mt19937& gen) : m_genome(n, 0), m_fitness(0) {

            std::iota(m_genome.begin(), m_genome.end(), 0);
            std::shuffle(m_genome.begin(), m_genome.end(), gen);

        }

        void update_fitness(const std::vector<std::vector<int>>& dist_matrix){

            int fit_count = 0;
            
            for (size_t i = 1; i < dist_matrix.size(); i++){
                
                int distance = dist_matrix[m_genome[i-1]][m_genome[i]];
                if (distance == -1){ 
                    //if the gene contains a unconnected edge apply a penalty
                    fit_count = fit_count + m_PENALTY;
                }
                else {
                    fit_count = fit_count + distance;
                }
                
            } 
            
            //add the return edge distance, check is its valid
            int return_edge = dist_matrix[m_genome.back()][m_genome[0]];
            if (return_edge == -1){
                m_fitness = fit_count + m_PENALTY;
            }
            else{
                m_fitness = fit_count + return_edge;
            }
            
        }
};

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
                m_population.emplace_back(m_N, m_gen);
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
