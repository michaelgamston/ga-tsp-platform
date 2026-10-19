#include <gtest/gtest.h>
#include <algorithm>
#include <map>
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

//! find_best_ind test -----------------------------------------------------------------

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

//! tournament selection test --------------------------------------------------------------------

// helper: the fitness of every individual in a group, in order
static std::vector<long long> fitnesses(const std::vector<Individual>& group){
    std::vector<long long> out;
    for (const Individual& ind : group) out.push_back(ind.get_fitness());
    return out;
}

// helper: a population of 5 with all-different fitness values
// MAP only has 3 distinct tours, so the two worst are built on a copy of MAP
// with edge 1 - 3 removed, which adds a penalty to any tour using it
static std::vector<Individual> population_of_five(){
    Matrix no_1_3 = MAP;
    no_1_3[1][3] = no_1_3[3][1] = -1;

    return {
        with_genome(MAP, MIDDLE),      // 45
        with_genome(no_1_3, WORST),    // penalty + 35   (worst)
        with_genome(MAP, BEST),        // 30             (best)
        with_genome(no_1_3, BEST),     // penalty + 14   (second worst)
        with_genome(MAP, WORST),       // 51
    };
}

static const long long P = Individual::m_PENALTY;
static const long long FIVE_BEST         = 30;
static const long long FIVE_SECOND_WORST = P + 14;
static const long long FIVE_WORST        = P + 35;

    // the pool is the same size as the population

TEST(TournamentSelection, PoolIsSameSizeAsPopulation) {
    auto pop = population_of_five();
    std::mt19937 gen(1);

    EXPECT_EQ(Genetic_algorithm::tournament_selection(pop, gen).size(), pop.size());
}

    // every member of the pool is a copy of someone in the population

TEST(TournamentSelection, EveryPoolMemberComesFromPopulation) {
    auto pop = population_of_five();
    std::mt19937 gen(1);

    auto pool = Genetic_algorithm::tournament_selection(pop, gen);

    for (const Individual& chosen : pool){
        bool found = std::any_of(pop.begin(), pop.end(), [&](const Individual& ind){
            return ind.get_genome() == chosen.get_genome() && ind.get_fitness() == chosen.get_fitness();
        });
        EXPECT_TRUE(found) << "pool member with fitness " << chosen.get_fitness() << " is not in the population";
    }
}

    // with 3 different competitors per tournament, the worst two can never win

TEST(TournamentSelection, WorstTwoAreNeverSelected) {
    auto pop = population_of_five();

    // try many seeds, since this must hold for every one of them
    for (unsigned seed = 0; seed < 50; seed++){
        std::mt19937 gen(seed);
        auto pool = Genetic_algorithm::tournament_selection(pop, gen);

        for (long long f : fitnesses(pool)){
            EXPECT_NE(f, FIVE_WORST)        << "seed " << seed;
            EXPECT_NE(f, FIVE_SECOND_WORST) << "seed " << seed;
        }
    }
}

    // when the population is exactly the tournament size, everyone competes every time,
    // so every slot in the pool is the best individual

TEST(TournamentSelection, PopulationOfTournamentSizeGivesAllBest) {
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, WORST), with_genome(MAP, BEST)};
    ASSERT_EQ(pop.size(), size_t(Genetic_algorithm::TOURNAMENT_SIZE));
    std::mt19937 gen(1);

    auto pool = Genetic_algorithm::tournament_selection(pop, gen);

    for (const Individual& chosen : pool){
        EXPECT_EQ(chosen.get_genome(), BEST);
    }
}

    // the same seed gives the same pool

TEST(TournamentSelection, SameSeedGivesSamePool) {
    auto pop = population_of_five();
    std::mt19937 gen_a(123);
    std::mt19937 gen_b(123);

    EXPECT_EQ(fitnesses(Genetic_algorithm::tournament_selection(pop, gen_a)),
              fitnesses(Genetic_algorithm::tournament_selection(pop, gen_b)));
}

    // selecting does not change the population

TEST(TournamentSelection, DoesNotModifyPopulation) {
    auto pop = population_of_five();
    auto before = fitnesses(pop);
    std::mt19937 gen(1);

    Genetic_algorithm::tournament_selection(pop, gen);

    EXPECT_EQ(fitnesses(pop), before);
}

    // statistical: the best individual is picked at least as often as any other

TEST(TournamentSelection, BestIsSelectedAtLeastAsOftenAsAnyOther) {
    auto pop = population_of_five();
    std::mt19937 gen(99);

    // count how many times each fitness value is chosen over many pools
    std::map<long long, int> counts;
    for (int run = 0; run < 200; run++){
        for (long long f : fitnesses(Genetic_algorithm::tournament_selection(pop, gen))){
            counts[f]++;
        }
    }

    for (const auto& [fitness, count] : counts){
        EXPECT_GE(counts[FIVE_BEST], count) << "fitness " << fitness << " was chosen more than the best";
    }
}

    // fail: an empty population has nobody to select

TEST(TournamentSelection, ThrowsOnEmptyPopulation) {
    std::vector<Individual> pop;
    std::mt19937 gen(1);

    EXPECT_THROW(Genetic_algorithm::tournament_selection(pop, gen), std::runtime_error);
}

    // fail: fewer individuals than the tournament size can't fill a tournament

TEST(TournamentSelection, ThrowsOnPopulationSmallerThanTournamentSize) {
    std::vector<Individual> pop = {with_genome(MAP, BEST), with_genome(MAP, WORST)};   // 2 < 3
    std::mt19937 gen(1);

    EXPECT_THROW(Genetic_algorithm::tournament_selection(pop, gen), std::runtime_error);
}


//! cross over test ------------------------------------------------------------------------------

// parents used by most tests (8 nodes, positions 0 - 7)
static const Genome A = {0, 1, 2, 3, 4, 5, 6, 7};
static const Genome B = {3, 7, 5, 1, 6, 0, 2, 4};
// cut points are inclusive: (2, 5) means positions 2, 3, 4 and 5


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

//! mutation tests ------------------------------------------------------------------------
// helper: a shuffled genome of n nodes
static Genome random_genome(int n, std::mt19937& rng){
    Genome g(n);
    std::iota(g.begin(), g.end(), 0);
    std::shuffle(g.begin(), g.end(), rng);
    return g;
}

// helper: true if `after` is `before` with exactly one block reversed and everything else untouched
static bool is_single_reversed_block(const Genome& before, const Genome& after){
    if (before.size() != after.size()) return false;

    // find the first and last positions that differ
    int first = -1, last = -1;
    for (int i = 0; i < int(before.size()); i++){
        if (before[i] != after[i]){
            if (first == -1) first = i;
            last = i;
        }
    }
    if (first == -1) return false;   // nothing changed

    // the block between them must be the original block in reverse order
    for (int i = first; i <= last; i++){
        if (after[i] != before[first + last - i]) return false;
    }
    return true;
}

    // rate 0 never changes the genome

TEST(Mutate, RateZeroNeverChangesGenome) {
    for (unsigned seed = 0; seed < 100; seed++){
        std::mt19937 gen(seed);
        Genome g = random_genome(10, gen);
        const Genome original = g;

        Genetic_algorithm::mutate(g, gen, 0.0f);

        EXPECT_EQ(g, original) << "seed " << seed;
    }
}

    // rate 1 always changes the genome (the two points always differ, so the
    // reversed block is at least 2 long and the order must change)

TEST(Mutate, RateOneAlwaysChangesGenome) {
    for (unsigned seed = 0; seed < 100; seed++){
        std::mt19937 gen(seed);
        Genome g = random_genome(10, gen);
        const Genome original = g;

        Genetic_algorithm::mutate(g, gen, 1.0f);

        EXPECT_NE(g, original) << "seed " << seed;
    }
}

    // the change is one block reversed, with everything outside it untouched

TEST(Mutate, ChangeIsASingleReversedBlock) {
    for (unsigned seed = 0; seed < 200; seed++){
        std::mt19937 gen(seed);
        Genome g = random_genome(12, gen);
        const Genome original = g;

        Genetic_algorithm::mutate(g, gen, 1.0f);

        EXPECT_TRUE(is_single_reversed_block(original, g)) << "seed " << seed;
    }
}

    // the result is always a valid permutation of the same length

TEST(Mutate, AlwaysReturnsValidPermutationOfSameLength) {
    std::mt19937 rng(7);

    for (int run = 0; run < 1000; run++){
        int n = std::uniform_int_distribution<int>(3, 50)(rng);
        float rate = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
        Genome g = random_genome(n, rng);

        Genetic_algorithm::mutate(g, rng, rate);

        ASSERT_EQ(g.size(), size_t(n)) << "run " << run;
        ASSERT_TRUE(is_valid_permutation(g, n)) << "run " << run << ", n = " << n << ", rate " << rate;
    }
}

    // the same seed gives the same result

TEST(Mutate, SameSeedGivesSameResult) {
    std::mt19937 setup(1);
    const Genome original = random_genome(20, setup);

    Genome a = original;
    Genome b = original;
    std::mt19937 gen_a(123);
    std::mt19937 gen_b(123);

    Genetic_algorithm::mutate(a, gen_a, 1.0f);
    Genetic_algorithm::mutate(b, gen_b, 1.0f);

    EXPECT_EQ(a, b);
}

    // the smallest valid genome (3 nodes, the map minimum) still mutates correctly

TEST(Mutate, SmallestGenomeStillMutatesCorrectly) {
    for (unsigned seed = 0; seed < 50; seed++){
        std::mt19937 gen(seed);
        Genome g = {0, 1, 2};
        const Genome original = g;

        Genetic_algorithm::mutate(g, gen, 1.0f);

        EXPECT_TRUE(is_valid_permutation(g, 3)) << "seed " << seed;
        EXPECT_TRUE(is_single_reversed_block(original, g)) << "seed " << seed;
    }
}

    // fail: mutation rate must be between 0 and 1

TEST(Mutate, ThrowsOnNegativeRate) {
    std::mt19937 gen(1);
    Genome g = {0, 1, 2, 3};

    EXPECT_THROW(Genetic_algorithm::mutate(g, gen, -0.1f), std::runtime_error);
}

TEST(Mutate, ThrowsOnRateAboveOne) {
    std::mt19937 gen(1);
    Genome g = {0, 1, 2, 3};

    EXPECT_THROW(Genetic_algorithm::mutate(g, gen, 1.1f), std::runtime_error);
}

    // fail: a genome too short to pick two different points

TEST(Mutate, ThrowsOnGenomeOfOneNode) {
    std::mt19937 gen(1);
    Genome g = {0};

    EXPECT_THROW(Genetic_algorithm::mutate(g, gen, 1.0f), std::runtime_error);
}

TEST(Mutate, ThrowsOnEmptyGenome) {
    std::mt19937 gen(1);
    Genome g;

    EXPECT_THROW(Genetic_algorithm::mutate(g, gen, 1.0f), std::runtime_error);
}
