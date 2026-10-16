#include <iostream>
#include <vector>
#include <random> 
#include <numeric>
#include <algorithm>
#include <string>
#include <sstream>
#include <stdexcept>

std::vector<std::vector<int>> parse_dist_matrix(std::istream& in){

    std::vector<std::vector<int>> dist_matrix; 
    std::string line; 
    size_t line_count = 0;

    while(std::getline(in, line)){
        line_count++;

        //skip any empty lines that might be present
        if (line.empty()) continue;

        std::vector<int> row;
        std::stringstream ss(line);
        std::string element; 
        size_t used = 0; 

        while(std::getline(ss, element, ',')){

            try{
                int value = std::stoi(element, &used);

                if (used == element.size()){
                    row.push_back(value);
                }
                else {
                    throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+" is of wrong type. Matrix must only contain whole numbers.");
                }
            }
            catch (const std::out_of_range&) {
                throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+ " is too large for int assignment.");
                
            }
            catch (const std::invalid_argument&) {
                throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+ " is of wrong type. Matrix must only contain whole numbers.");
                
            }
            
        }

        dist_matrix.push_back(row);

    }

    //check it is square
    // .size() returns a size_t so keeping the convention to avoid compiler erros
    size_t height = dist_matrix.size();
    for (size_t i = 0; i < height; i++){

        // row i, where i counts from 0 and skips blank lines. 
        // it won't necessarily match line_count
        // so the user could be pointed to the wrong line. 

        if (dist_matrix[i].size() != height){
            
            throw std::runtime_error("Matrix is not square: row " + std::to_string(i) +
                                 " has " + std::to_string(dist_matrix[i].size()) +
                                 " values, expected " + std::to_string(height));
        } 
    }

    return dist_matrix;
}

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
