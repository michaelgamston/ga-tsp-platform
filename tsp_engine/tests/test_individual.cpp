#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>
#include "tsp_engine/individual.h"

using Matrix = std::vector<std::vector<int>>;

// shorter name for the penalty used in expected values
static constexpr long long PENALTY = Individual::m_PENALTY;

// fully connected 4 node map, small enough to check tour lengths by hand
//        0   1   2   3
// 0  [   0, 10, 15, 20 ]
// 1  [  10,  0, 35, 25 ]
// 2  [  15, 35,  0, 30 ]
// 3  [  20, 25, 30,  0 ]
static const Matrix FULL = {
    { 0, 10, 15, 20},
    {10,  0, 35, 25},
    {15, 35,  0, 30},
    {20, 25, 30,  0},
};

// helper: build an individual, then give it a chosen tour
static Individual with_genome(const Matrix& m, const std::vector<int>& genome){
    std::mt19937 gen(0);
    Individual ind(gen, m);
    ind.update_m_genome(genome, m);
    return ind;
}

// pass test

    // check genome and matrix are of the same size

TEST(Individual, GenomeSizeMatchesMatrix) {
    std::mt19937 gen(42);
    Individual ind(gen, FULL);

    EXPECT_EQ(ind.get_m_genome().size(), FULL.size());
}

    // check genome contains only unique values

TEST(Individual, GenomeIsValidPermutation) {
    std::mt19937 gen(42);
    Individual ind(gen, FULL);

    // sorting a valid permutation of n nodes must give 0, 1, ..., n-1
    std::vector<int> sorted = ind.get_m_genome();   // a copy, so sorting doesn't touch the individual
    std::sort(sorted.begin(), sorted.end());

    std::vector<int> expected(FULL.size());
    std::iota(expected.begin(), expected.end(), 0);

    EXPECT_EQ(sorted, expected);
}

    // check fitness is calculated on construction

TEST(Individual, FitnessCalculatedOnConstruction) {
    std::mt19937 gen(42);
    Individual ind(gen, FULL);

    // work out the length of whatever tour it was given, independently
    long long expected = 0;
    const auto& g = ind.get_m_genome();
    for (size_t i = 0; i < g.size(); i++){
        expected += FULL[g[i]][g[(i + 1) % g.size()]];   // % wraps the last node back to the first
    }

    EXPECT_EQ(ind.get_fitness(), expected);
}

    // check two seperate generator calls with the same seed produce the same genomes

TEST(Individual, SameSeedGivesSameGenomes) {
    std::mt19937 gen_a(123);
    std::mt19937 gen_b(123);

    // the first individual from each generator should match...
    Individual a1(gen_a, FULL);
    Individual b1(gen_b, FULL);
    EXPECT_EQ(a1.get_m_genome(), b1.get_m_genome());

    // ...and so should the next, since both generators advance identically
    Individual a2(gen_a, FULL);
    Individual b2(gen_b, FULL);
    EXPECT_EQ(a2.get_m_genome(), b2.get_m_genome());
}

    // check the fitness is calculated correctly for a known route

TEST(Individual, KnownRouteHasCorrectFitness) {
    // 0 -> 1 -> 2 -> 3 -> back to 0  =  10 + 35 + 30 + 20
    EXPECT_EQ(with_genome(FULL, {0, 1, 2, 3}).get_fitness(), 95);

    // 0 -> 1 -> 3 -> 2 -> back to 0  =  10 + 25 + 30 + 15
    EXPECT_EQ(with_genome(FULL, {0, 1, 3, 2}).get_fitness(), 80);
}

    // check that two routes with the same path have the same fitness regardless of starting point

TEST(Individual, StartingPointDoesNotChangeFitness) {
    long long from_0 = with_genome(FULL, {0, 1, 2, 3}).get_fitness();

    EXPECT_EQ(with_genome(FULL, {1, 2, 3, 0}).get_fitness(), from_0);
    EXPECT_EQ(with_genome(FULL, {2, 3, 0, 1}).get_fitness(), from_0);
    EXPECT_EQ(with_genome(FULL, {3, 0, 1, 2}).get_fitness(), from_0);
}

    // check fitness is the same for two routes with the same path but one path being reversed

TEST(Individual, ReversedRouteHasSameFitness) {
    EXPECT_EQ(with_genome(FULL, {0, 1, 2, 3}).get_fitness(),
              with_genome(FULL, {0, 3, 2, 1}).get_fitness());
}

    // check a route with -1 within it genome is given a penalty

TEST(Individual, UnconnectedEdgeAddsPenalty) {
    Matrix m = FULL;
    m[1][2] = m[2][1] = -1;

    // 0 -> 1 (10), 1 -> 2 (no edge), 2 -> 3 (30), 3 -> 0 (20)
    EXPECT_EQ(with_genome(m, {0, 1, 2, 3}).get_fitness(), 60 + PENALTY);
}

    // check multiple -1's receive an equal number of penaltys

TEST(Individual, EachUnconnectedEdgeAddsOnePenalty) {
    Matrix m = FULL;
    m[1][2] = m[2][1] = -1;
    m[2][3] = m[3][2] = -1;

    // 0 -> 1 (10), 1 -> 2 (no edge), 2 -> 3 (no edge), 3 -> 0 (20)
    EXPECT_EQ(with_genome(m, {0, 1, 2, 3}).get_fitness(), 30 + 2 * PENALTY);
}

    // check a -1 on the return edge is given a penalty

TEST(Individual, UnconnectedReturnEdgeAddsPenalty) {
    Matrix m = FULL;
    m[3][0] = m[0][3] = -1;

    // 0 -> 1 (10), 1 -> 2 (35), 2 -> 3 (30), 3 -> back to 0 (no edge)
    EXPECT_EQ(with_genome(m, {0, 1, 2, 3}).get_fitness(), 75 + PENALTY);
}

    // changing the genome with update_m_genome successfully updates the fitness

TEST(Individual, UpdateGenomeRecalculatesFitness) {
    Individual ind = with_genome(FULL, {0, 1, 2, 3});
    ASSERT_EQ(ind.get_fitness(), 95);

    ind.update_m_genome({0, 1, 3, 2}, FULL);

    EXPECT_EQ(ind.get_fitness(), 80);
}
