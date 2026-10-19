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
#include "tsp_engine/genetic_algorithm.h"


Genetic_algorithm::Genetic_algorithm(int p, int r, float m, std::istream& in) : m_P(p), m_R(r), m_M(m), m_dist_matrix(parse_dist_matrix(in)){
    
    //generate new population
    //reserve space in the vector 
    m_population.reserve(m_P);
    for (int i = 0; i < m_P; i++){
        //in each empty space run the Individual constructor
        m_population.emplace_back(m_gen, m_dist_matrix);
    }


    //hello world
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

