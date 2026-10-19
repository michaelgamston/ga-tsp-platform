#include <vector>
#include <random> 
#include <algorithm>
#include <string>
#include <stdexcept>
#include <cassert>
#include <numeric>
#include <iterator>
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

Individual Genetic_algorithm::find_best_ind(const std::vector<Individual>& population){

    if (population.size() == 0u) throw std::runtime_error("Find best ind Error : population empty");

    //to avoid copying each time we can point to the best element
    const Individual* best_ind_ptr = &population[0];

    for (const Individual& ind : population){
        //iterating through each ind in population
        //if a new lowest fitness is found the pointer shifts to point at that ind
        if (ind.get_fitness() < best_ind_ptr->get_fitness()){
            best_ind_ptr = &ind;
        }
    }

    //dereference the pointer and return the object
    return *best_ind_ptr;

}

std::vector<Individual> Genetic_algorithm::tournament_selection(const std::vector<Individual>& population, std::mt19937& gen){
    
    if (int(population.size()) < TOURNAMENT_SIZE) throw std::runtime_error("Tournament selection Error : population size is lower than "+std::to_string(TOURNAMENT_SIZE));

    int competitorA; 
    int competitorB;
    std::uniform_int_distribution<int> pick(0, int(population.size()) - 1);

    std::vector<Individual> mating_pool;
    mating_pool.reserve(population.size());

    for (int i = 0; i < int(population.size()); i++){
        competitorA = 0;
        competitorB = 0;
        // pick unique competitors
        while (competitorA == competitorB || competitorA == i || competitorB == i){
            competitorA = pick(gen);
            competitorB = pick(gen);
        }

        int best = i;                                      
        for (int c : {competitorA, competitorB}) {         
            if (population[c].get_fitness() < population[best].get_fitness()) {
                best = c;
            }
        }
        mating_pool.push_back(population[best]);           


    }

    return mating_pool;

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

void Genetic_algorithm::mutate(std::vector<int>& genome, std::mt19937& gen, const float mutation_rate){

    if (mutation_rate < 0) throw std::runtime_error("Mutate Error : mutation rate is negative.");
    if (mutation_rate > 1) throw std::runtime_error("Mutate Error : mutation rate is over 1.");
    if (genome.size() < 2) throw std::runtime_error("Mutate Error : genome of one gene dectected");

    std::bernoulli_distribution should_mutate(mutation_rate);

    if (should_mutate(gen)){

        std::vector<int> positions(genome.size());
        std::iota(positions.begin(), positions.end(), 0);        // 0, 1, 2, ..., n-1

        std::vector<int> points;
        std::sample(positions.begin(), positions.end(), std::back_inserter(points), 2, gen);

        int low  = points[0];   // always the smaller one: sample keeps the input order
        int high = points[1];   // always different from low: no repeats

        std::reverse(genome.begin() + low, genome.begin() + high + 1);

    }
}

