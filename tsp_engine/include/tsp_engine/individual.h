#pragma once
#include <vector>
#include <random>

class Individual{
    private:

        std::vector<int> m_genome; 
        long long m_fitness; 

        void update_fitness(const std::vector<std::vector<int>>& dist_matrix);

    public: 

        static constexpr int m_PENALTY = 1000000;

        Individual(std::mt19937& gen, const std::vector<std::vector<int>>& dist_matrix);

        long long get_fitness() const;

        void update_genome(std::vector<int> new_genome, const std::vector<std::vector<int>>& dist_matrix);

        const std::vector<int>& get_genome() const;


};