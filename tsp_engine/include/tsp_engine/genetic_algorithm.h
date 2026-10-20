#pragma once
#include <vector>
#include <random> 
#include <algorithm>
#include <string>
#include <stdexcept>
#include <optional>
#include "tsp_engine/map_io.h"
#include "tsp_engine/individual.h"

class Genetic_algorithm{

    private: 
       
        const int m_P; //population size
        const int m_R; // iterations 
        const float m_M; // mutation rate  
       
        std::vector<Individual> m_population; // for storing the population of individuals
        std::vector<std::vector<int>> m_dist_matrix; // matrix storing distances

        //random number generator 
        std::mt19937 m_gen;

    public: 

        struct GA_result {
            Individual best; 
            std::vector<long long> best_fit_per_generation;

            explicit GA_result(const Individual& initial_best) : best(initial_best) {}
        };

        static constexpr int TOURNAMENT_SIZE = 3;
        
        Genetic_algorithm(int p, int r, float m, std::istream& in, unsigned seed);

        //find current best indiviual
        static Individual find_best_ind(const std::vector<Individual>& population);

        //tornament selection 
            //randomly select two parents and keep the strongest as an offspring
        static std::vector<Individual> tournament_selection(const std::vector<Individual>& population, std::mt19937& gen);

        // go through offspring in pairs and cross over the genomes
        static std::vector<int> crossover(const std::vector<int>& genome1,const std::vector<int>& genome2, const int cross_low, const int cross_high);

        // randomly mutate (or swap) genes in a genome 
        static void mutate(std::vector<int>& genome, std::mt19937& gen, const float mutation_rate);

        //swap the best individual from the last run for the worst of this run
        static void elitism(std::vector<Individual>& population, const Individual& previous_best);

        //write progress per generation to output
        static void write_progress(std::ostream& out, int generation, const Individual& best);

        GA_result run_ga(std::ostream& out);
        //build a runner function 
            //runs the algorithm for set number of itterations 
            // or untill it stops improving
};