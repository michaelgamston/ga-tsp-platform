#include "tsp_engine/individual.h"
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <string>
#include <sstream>

//using the intialiser list to construct the variables with elements
//rather than contructing them emmpty and then assigning them
Individual::Individual(std::mt19937& gen, const std::vector<std::vector<int>>& dist_matrix) : m_genome(int(dist_matrix.size()), 0), m_fitness(0) {

        // fill genome with nodes and then shuffle it to create random starting series
        std::iota(m_genome.begin(), m_genome.end(), 0);
        std::shuffle(m_genome.begin(), m_genome.end(), gen);

        //update starting fitness
        update_fitness(dist_matrix);


    }

void Individual::update_fitness(const std::vector<std::vector<int>>& dist_matrix){

    long long fit_count = 0;
    
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

long long Individual::get_fitness() const{
    return m_fitness;
}

void Individual::update_genome(std::vector<int> new_genome, const std::vector<std::vector<int>>& dist_matrix){
    
    int num_nodes = int(dist_matrix.size());
    std::vector<bool> seen((m_genome.size()), false);

    // checks for valid genome structure 
    //! make this debug only in future fix
    if (new_genome.size() != dist_matrix.size()){
        throw std::runtime_error("Genome is incorrec size. Should be = "+std::to_string(dist_matrix.size())+
        " actual size is = "+std::to_string(new_genome.size()));
    }

    for (int node : new_genome){
        if (node < 0 || node > num_nodes-1 || seen[node]){

            std::ostringstream out;
            for (size_t i = 0; i < new_genome.size(); i++) {
                if (i > 0) out << ", ";
                out << new_genome[i];
            }
            std::string text = out.str();   // e.g. "0, 1, 1, 3"

            throw std::runtime_error("Genome has incorrect values : " +text);
        }
        seen[node] = true;
    }

    m_genome = std::move(new_genome);
    update_fitness(dist_matrix);
}

const std::vector<int>& Individual::get_genome() const{
    return m_genome;
}
