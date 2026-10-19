#pragma once
#include <vector>
#include <random> 
#include <algorithm>
#include <string>
#include <stdexcept>
#include "tsp_engine/map_io.h"
#include "tsp_engine/individual.h"

class Genetic_algorithm{

    private: 
        //! review variables, its changed and I wont need them all

        const int m_P; //population size
        const int m_R; // terations 
        const float m_M; // mutation rate  
        Individual best_ind;
       
        std::vector<int> m_best; // for storung the best result
        std::vector<Individual> m_population; // for storing the population of individuals
        std::vector<std::vector<int>> m_dist_matrix; // matrix storing distances


        //random number generator 
        std::mt19937 m_gen;

        


    public: 

        
        Genetic_algorithm(int p, int r, float m, std::istream& in, unsigned seed);

        //find current best indiviual
        static Individual find_best_ind(const std::vector<Individual>& population);

        //tornament selection 
            //randomly select two parents and keep the strongest as an offspring
        static Individual& tournament_selection(Individual& offspring1, Individual& offspring2);

        // go through offspring in pairs and cross over the genomes
        static std::vector<int> crossover(const std::vector<int>& genome1,const std::vector<int>& genome2, const int cross_low, const int cross_high);

        // randomly mutate (or swap) genes in a genome 
        static void mutate(Individual& offspring);

        //swap the best individual from the last run for the worst of this run
        void elitism();

        void run_ga();
        //build a runner function 
            //runs the algorithm for set number of itterations 
            // or untill it stops improving

};