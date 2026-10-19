#include <vector>
#include <random> 
#include <algorithm>
#include <string>
#include <stdexcept>
#include <cassert>
#include "tsp_engine/map_io.h"
#include "tsp_engine/individual.h"
#include "tsp_engine/genetic_algorithm.h"


Genetic_algorithm::Genetic_algorithm(int p, int r, float m, std::istream& in, unsigned seed) 
: m_P(p), m_R(r), m_M(m), m_dist_matrix(parse_dist_matrix(in)), m_gen(seed){
    
    //generate new population
    //reserve space in the vector 
    m_population.reserve(m_P);
    for (int i = 0; i < m_P; i++){
        //in each empty space run the Individual constructor
        m_population.emplace_back(m_gen, m_dist_matrix);
    }


}    

std::vector<int> Genetic_algorithm::crossover(const std::vector<int>& genome1,const std::vector<int>& genome2, const int cross_low, const int cross_high){
    
    if (genome1.size() != genome2.size()) throw std::runtime_error("Crossover Error : genomes of different lengths found in crossover");
    if (cross_low > cross_high) throw std::runtime_error("Crossover Error : highcross point is lower than lowcross point");
    if (cross_low < 0) throw std::runtime_error("Crossover Error : lowcross point is negative");
    if (cross_high >= int(genome1.size())) throw std::runtime_error("Crossover Error : highcross point out of bounds");

    std::vector<int> offspring(genome1.size());
    std::vector<bool> used(genome1.size());

    for (int i = cross_low; i <= cross_high; i++){
        offspring[i] = genome1[i];
        used[genome1[i]] = true;
    }

    int off_ptr = 0;

    for (int node : genome2){

        if (!used[node]){
            if (off_ptr == cross_low) off_ptr = cross_high+1;
            offspring[off_ptr] = node;
            off_ptr++;
        }

    }
    
    return offspring;

}

//find current best indiviual

//tornament selection 
    //randomly select two parents and keep the strongest as an offspring

// go through offspring in pairs and cross over the genomes

// randomly mutate (or swap) genes in a genome 

//swap the best individual from the last run for the worst of this run

//build a runner function 
    //runs the algorithm for set number of itterations 
    // or untill it stops improving

