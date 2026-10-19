#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>
#include "tsp_engine/genetic_algorithm.h"

using Genome = std::vector<int>;

// helper: true if g contains every node 0 .. n-1 exactly once
static bool is_valid_permutation(const Genome& g, size_t n){
    if (g.size() != n) return false;
    Genome sorted = g;
    std::sort(sorted.begin(), sorted.end());
    Genome expected(n);
    std::iota(expected.begin(), expected.end(), 0);
    return sorted == expected;
}

// find_best_ind test

using Matrix = std::vector<std::vector<int>>;

// 4 node map where the 3 possible tours all have different lengths
//        0   1   2   3
// 0  [   0,  1,  2,  4 ]
// 1  [   1,  0,  8, 16 ]
// 2  [   2,  8,  0, 32 ]
// 3  [   4, 16, 32,  0 ]
static const Matrix MAP = {
    {0,  1,  2,  4},
    {1,  0,  8, 16},
    {2,  8,  0, 32},
    {4, 16, 32,  0},
};

static const Genome BEST   = {0, 2, 1, 3};   // 2 + 8 + 16 + 4  = 30
static const Genome MIDDLE = {0, 1, 2, 3};   // 1 + 8 + 32 + 4  = 45
static const Genome WORST  = {0, 1, 3, 2};   // 1 + 16 + 32 + 2 = 51

// helper: build an individual with a chosen tour on a given map
static Individual with_genome(const Matrix& m, const Genome& genome){
    std::mt19937 gen(0);
    Individual ind(gen, m);
    ind.update_genome(genome, m);
    return ind;
}

    // ensure best individual is found based on having the lowest fitness

TEST(FindBestInd, ReturnsLowestFitness) {
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, BEST), with_genome(MAP, WORST)};

    Individual best = Genetic_algorithm::find_best_ind(pop);

    EXPECT_EQ(best.get_fitness(), 30);
    EXPECT_EQ(best.get_genome(), BEST);
}

    // best individual is the first in the population (boundary)

TEST(FindBestInd, FindsBestInFirstPosition) {
    std::vector<Individual> pop = {with_genome(MAP, BEST), with_genome(MAP, MIDDLE), with_genome(MAP, WORST)};

    EXPECT_EQ(Genetic_algorithm::find_best_ind(pop).get_genome(), BEST);
}

    // best individual is the last in the population (boundary)

TEST(FindBestInd, FindsBestInLastPosition) {
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, WORST), with_genome(MAP, BEST)};

    EXPECT_EQ(Genetic_algorithm::find_best_ind(pop).get_genome(), BEST);
}

    // a population of one returns that individual

TEST(FindBestInd, SingleIndividualIsReturned) {
    std::vector<Individual> pop = {with_genome(MAP, WORST)};

    EXPECT_EQ(Genetic_algorithm::find_best_ind(pop).get_genome(), WORST);
}

    // two individuals tied for best: either one is acceptable, but it must be one of them

TEST(FindBestInd, TiedBestReturnsOneOfThem) {
    // {2, 1, 3, 0} is the same cycle as BEST from a different start, so also 30
    const Genome BEST_ROTATED = {2, 1, 3, 0};
    std::vector<Individual> pop = {with_genome(MAP, WORST), with_genome(MAP, BEST),
                                   with_genome(MAP, MIDDLE), with_genome(MAP, BEST_ROTATED)};

    Individual best = Genetic_algorithm::find_best_ind(pop);

    EXPECT_EQ(best.get_fitness(), 30);
    EXPECT_TRUE(best.get_genome() == BEST || best.get_genome() == BEST_ROTATED);
}

    // a short tour using an unconnected edge is never chosen over a longer valid one

TEST(FindBestInd, PenalisedTourIsNotChosenOverValidTour) {
    Matrix m = MAP;
    m[1][3] = m[3][1] = -1;

    // ignoring the penalty, BEST would be shortest (2 + 8 - 1 + 4 = 13),
    // but it uses edge 1 - 3, so MIDDLE (45, all edges valid) should win
    std::vector<Individual> pop = {with_genome(m, BEST), with_genome(m, MIDDLE), with_genome(m, WORST)};

    Individual best = Genetic_algorithm::find_best_ind(pop);

    EXPECT_EQ(best.get_genome(), MIDDLE);
    EXPECT_EQ(best.get_fitness(), 45);
}

    // finding the best does not change the population

TEST(FindBestInd, DoesNotModifyPopulation) {
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, BEST), with_genome(MAP, WORST)};

    Genetic_algorithm::find_best_ind(pop);

    ASSERT_EQ(pop.size(), 3u);
    EXPECT_EQ(pop[0].get_genome(), MIDDLE);
    EXPECT_EQ(pop[1].get_genome(), BEST);
    EXPECT_EQ(pop[2].get_genome(), WORST);
}

    // fail: an empty population has no best individual

TEST(FindBestInd, ThrowsOnEmptyPopulation) {
    std::vector<Individual> pop;

    EXPECT_THROW(Genetic_algorithm::find_best_ind(pop), std::runtime_error);
}


// parents used by most tests (8 nodes, positions 0 - 7)
static const Genome A = {0, 1, 2, 3, 4, 5, 6, 7};
static const Genome B = {3, 7, 5, 1, 6, 0, 2, 4};
// cut points are inclusive: (2, 5) means positions 2, 3, 4 and 5

// cross over test

    // pass
        // return a child with crossover in the correct range
        //   the slice between the cut points matches parent A

TEST(Crossover, SliceBetweenCutPointsMatchesParentA) {
    Genome child = Genetic_algorithm::crossover(A, B, 2, 5);

    ASSERT_EQ(child.size(), A.size());
    for (int i = 2; i <= 5; i++){
        EXPECT_EQ(child[i], A[i]) << "position " << i;
    }
}

        // returns a valid permutation
        //   check over many random runs (e.g. 1,000 random parents + cut points)

TEST(Crossover, AlwaysReturnsValidPermutation) {
    std::mt19937 rng(7);   // fixed seed, so any failure can be reproduced

    for (int run = 0; run < 1000; run++){
        // random genome length between 3 and 50 (the map size limits)
        int n = std::uniform_int_distribution<int>(3, 50)(rng);

        Genome a(n), b(n);
        std::iota(a.begin(), a.end(), 0);
        std::iota(b.begin(), b.end(), 0);
        std::shuffle(a.begin(), a.end(), rng);
        std::shuffle(b.begin(), b.end(), rng);

        std::uniform_int_distribution<int> pick(0, n - 1);
        int p1 = pick(rng);
        int p2 = pick(rng);
        auto [start, end] = std::minmax(p1, p2);

        Genome child = Genetic_algorithm::crossover(a, b, start, end);

        ASSERT_TRUE(is_valid_permutation(child, n))
            << "run " << run << ", n = " << n << ", cut points " << start << "-" << end;
        for (int i = start; i <= end; i++){
            ASSERT_EQ(child[i], a[i]) << "run " << run << ", position " << i;
        }
    }
}

        // slice covering the whole genome returns a copy of parent A

TEST(Crossover, WholeGenomeSliceReturnsCopyOfParentA) {
    EXPECT_EQ(Genetic_algorithm::crossover(A, B, 0, 7), A);
}

        // slice of a single city is still a valid child with that city in place

TEST(Crossover, SingleCitySliceKeepsThatCity) {
    Genome child = Genetic_algorithm::crossover(A, B, 3, 3);

    EXPECT_TRUE(is_valid_permutation(child, A.size()));
    EXPECT_EQ(child[3], A[3]);
}

        // slice starting at position 0

TEST(Crossover, SliceStartingAtFirstPosition) {
    Genome child = Genetic_algorithm::crossover(A, B, 0, 3);

    EXPECT_TRUE(is_valid_permutation(child, A.size()));
    for (int i = 0; i <= 3; i++){
        EXPECT_EQ(child[i], A[i]) << "position " << i;
    }
}

        // slice ending at the last position

TEST(Crossover, SliceEndingAtLastPosition) {
    Genome child = Genetic_algorithm::crossover(A, B, 4, 7);

    EXPECT_TRUE(is_valid_permutation(child, A.size()));
    for (int i = 4; i <= 7; i++){
        EXPECT_EQ(child[i], A[i]) << "position " << i;
    }
}

    // fail

        // parents of different lengths are rejected

TEST(Crossover, ThrowsOnParentsOfDifferentLengths) {
    Genome shorter = {3, 1, 0, 2, 6, 5, 4};   // 7 nodes vs A's 8

    EXPECT_THROW(Genetic_algorithm::crossover(A, shorter, 2, 5), std::runtime_error);
}

        // cut points out of range are rejected

TEST(Crossover, ThrowsOnEndPastLastPosition) {
    // A has positions 0 - 7, so 8 is just out of range (boundary test)
    EXPECT_THROW(Genetic_algorithm::crossover(A, B, 2, 8), std::runtime_error);
}

TEST(Crossover, ThrowsOnBothCutPointsOutOfRange) {
    EXPECT_THROW(Genetic_algorithm::crossover(A, B, 10, 12), std::runtime_error);
}

        // if highcross over point is lower than the lowcross point 

TEST(Crossover, ThrowsOnHighcrossLowerThanLowcross){
    EXPECT_THROW(Genetic_algorithm::crossover(A, B, 5, 4), std::runtime_error);
}

        // if lowcross point is negative

TEST(Crossover, ThrowsOnNegativeLowcross){
    EXPECT_THROW(Genetic_algorithm::crossover(A, B, -1, 6), std::runtime_error);
}
