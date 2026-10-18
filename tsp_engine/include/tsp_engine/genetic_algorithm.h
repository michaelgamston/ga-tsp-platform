#pragma once
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
        //! review variables, its changed and I wont need them all

        //random number generator 
        std::mt19937 m_gen{std::random_device{}()};

        const int m_P; //population size
        const int m_R; // terations 
        const float m_M; // mutation rate  

       
        std::vector<int> m_best; // for storung the best result
        std::vector<Individual> m_population; // for storing the population of individuals
        std::vector<std::vector<int>> m_dist_matrix; // matrix storing distances


        //find current best indiviual
        std::vector<int> find_best_ind();


        //tornament selection 
            //randomly select two parents and keep the strongest as an offspring
        void tournament_selectin();

        // go through offspring in pairs and cross over the genomes
        void crossover();

        // randomly mutate (or swap) genes in a genome 
        void mutate();

        //swap the best individual from the last run for the worst of this run
        void elitism();


    public: 

        
        Genetic_algorithm(int p, int r, float m, std::istream& in);

        void run_ga();
        //build a runner function 
            //runs the algorithm for set number of itterations 
            // or untill it stops improving

};