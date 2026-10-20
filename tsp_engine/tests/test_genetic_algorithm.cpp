#include <gtest/gtest.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
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

//! elitism tests -------------------------------------------------------------

// a map where every edge is 5, so every tour is 20: better than anything on MAP
static const Matrix FLAT = {
    {0, 5, 5, 5},
    {5, 0, 5, 5},
    {5, 5, 0, 5},
    {5, 5, 5, 0},
};

// the previous generation's best: fitness 20, and a genome none of the MAP individuals use
static const Genome ELITE_GENOME = {3, 2, 1, 0};
static Individual elite(){ return with_genome(FLAT, ELITE_GENOME); }   // 5 + 5 + 5 + 5 = 20

// pass

    // the worst individual is replaced by the previous best

TEST(Elitism, WorstIsReplacedByPreviousBest) {
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, WORST), with_genome(MAP, BEST)};

    Genetic_algorithm::elitism(pop, elite());

    // WORST (51) was at index 1
    EXPECT_EQ(pop[1].get_genome(), ELITE_GENOME);
    EXPECT_EQ(pop[1].get_fitness(), 20);
}

    // everyone other than the worst is left unchanged

TEST(Elitism, OtherIndividualsAreUnchanged) {
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, WORST), with_genome(MAP, BEST)};

    Genetic_algorithm::elitism(pop, elite());

    EXPECT_EQ(pop[0].get_genome(), MIDDLE);
    EXPECT_EQ(pop[0].get_fitness(), 45);
    EXPECT_EQ(pop[2].get_genome(), BEST);
    EXPECT_EQ(pop[2].get_fitness(), 30);
}

    // the population size does not change

TEST(Elitism, PopulationSizeIsUnchanged) {
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, WORST), with_genome(MAP, BEST)};

    Genetic_algorithm::elitism(pop, elite());

    EXPECT_EQ(pop.size(), 3u);
}

    // the worst is the first in the population (boundary)

TEST(Elitism, ReplacesWorstInFirstPosition) {
    std::vector<Individual> pop = {with_genome(MAP, WORST), with_genome(MAP, MIDDLE), with_genome(MAP, BEST)};

    Genetic_algorithm::elitism(pop, elite());

    EXPECT_EQ(pop[0].get_genome(), ELITE_GENOME);
    EXPECT_EQ(pop[1].get_genome(), MIDDLE);
    EXPECT_EQ(pop[2].get_genome(), BEST);
}

    // the worst is the last in the population (boundary)

TEST(Elitism, ReplacesWorstInLastPosition) {
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, BEST), with_genome(MAP, WORST)};

    Genetic_algorithm::elitism(pop, elite());

    EXPECT_EQ(pop[0].get_genome(), MIDDLE);
    EXPECT_EQ(pop[1].get_genome(), BEST);
    EXPECT_EQ(pop[2].get_genome(), ELITE_GENOME);
}

    // when two individuals tie for worst, exactly one of them is replaced

TEST(Elitism, TiedWorstReplacesExactlyOne) {
    std::vector<Individual> pop = {with_genome(MAP, WORST), with_genome(MAP, BEST), with_genome(MAP, WORST)};

    Genetic_algorithm::elitism(pop, elite());

    int still_worst = 0;
    int elites = 0;
    for (const Individual& ind : pop){
        if (ind.get_fitness() == 51) still_worst++;
        if (ind.get_genome() == ELITE_GENOME) elites++;
    }
    EXPECT_EQ(still_worst, 1);
    EXPECT_EQ(elites, 1);
    EXPECT_EQ(pop[1].get_genome(), BEST);   // the non-tied individual is untouched
}

    // if the previous best has a worse fitness than current worst expect it not to be swapped

TEST(Elitism, PreviousBestNotSwappedIfWorseThanCurrentWorst){
    std::vector<Individual> pop = {with_genome(MAP, MIDDLE), with_genome(MAP, BEST), elite()};

    Genetic_algorithm::elitism(pop, with_genome(MAP, WORST));

    int swapped = 0;
    for (const Individual& ind : pop){
        if (ind.get_genome() == WORST){
            swapped = 1;
        }
    }

    EXPECT_EQ(swapped, 0);
}

//fail

    // fail: an empty population has no worst individual to replace

TEST(Elitism, ThrowsOnEmptyPopulation) {
    std::vector<Individual> pop;

    EXPECT_THROW(Genetic_algorithm::elitism(pop, elite()), std::runtime_error);
}


    //fail: a population size of one cannot evole and must no be accecpted 

TEST(Elitism, ThrowsOnPopulationOfOne){
    
    std::vector<Individual> pop = {with_genome(MAP, WORST)};
    EXPECT_THROW(Genetic_algorithm::elitism(pop, elite()), std::runtime_error);

}
//! run_ga tests ---------------------------------------------------------------------------

// maps/ sits two folders above this test file (tests/ -> tsp_engine/ -> repo root)
static std::string ga_maps_dir(){
    std::string here = __FILE__;
    return here.substr(0, here.find_last_of('/')) + "/../../maps/";
}

// helper: load a map file's distance matrix (to check results against)
static Matrix load_map(const std::string& name){
    std::ifstream file(ga_maps_dir() + name);
    return parse_dist_matrix(file);
}

// helper: tour length worked out independently of Individual, including penalties
static long long tour_length(const Matrix& m, const Genome& g){
    long long total = 0;
    for (size_t i = 0; i < g.size(); i++){
        int d = m[g[i]][g[(i + 1) % g.size()]];   // % wraps the last node back to the first
        total += (d == -1) ? Individual::m_PENALTY : d;
    }
    return total;
}

// helper: build a GA from a map file and run it
// progress lines go into `progress` (pass nothing if a test doesn't need them)
static Genetic_algorithm::GA_result run_on(const std::string& name, int population, int generations,
                                           float mutation_rate, unsigned seed,
                                           std::ostream* progress = nullptr){
    std::ifstream file(ga_maps_dir() + name);
    Genetic_algorithm ga(population, generations, mutation_rate, file, seed);

    std::ostringstream ignored;   // somewhere to send output when the test doesn't look at it
    return ga.run_ga(progress ? *progress : ignored);
}

// helper: split captured output into its lines
static std::vector<std::string> lines_of(const std::string& text){
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) lines.push_back(line);
    return lines;
}

    // fail: constructor rejects settings run_ga can't work with

TEST(GeneticAlgorithm, ThrowsOnPopulationSmallerThanTournamentSize) {
    std::ifstream file(ga_maps_dir() + "map_05.csv");
    EXPECT_THROW(Genetic_algorithm(Genetic_algorithm::TOURNAMENT_SIZE - 1, 10, 0.1f, file, 1), std::runtime_error);
}

TEST(GeneticAlgorithm, ThrowsOnZeroGenerations) {
    std::ifstream file(ga_maps_dir() + "map_05.csv");
    EXPECT_THROW(Genetic_algorithm(10, 0, 0.1f, file, 1), std::runtime_error);
}

TEST(GeneticAlgorithm, ThrowsOnNegativeMutationRate) {
    std::ifstream file(ga_maps_dir() + "map_05.csv");
    EXPECT_THROW(Genetic_algorithm(10, 10, -0.1f, file, 1), std::runtime_error);
}

TEST(GeneticAlgorithm, ThrowsOnMutationRateAboveOne) {
    std::ifstream file(ga_maps_dir() + "map_05.csv");
    EXPECT_THROW(Genetic_algorithm(10, 10, 1.1f, file, 1), std::runtime_error);
}

    // the best fitness never gets worse from one generation to the next (guaranteed by elitism)

TEST(RunGA, BestFitnessNeverGetsWorse) {
    auto result = run_on("map_08.csv", 30, 100, 0.1f, 42);

    for (size_t gen = 1; gen < result.best_fit_per_generation.size(); gen++){
        EXPECT_LE(result.best_fit_per_generation[gen], result.best_fit_per_generation[gen - 1])
            << "best got worse at generation " << gen;
    }
}

    // runs exactly the requested number of generations

TEST(RunGA, RunsRequestedNumberOfGenerations) {
    auto result = run_on("map_05.csv", 20, 37, 0.1f, 42);

    EXPECT_EQ(result.best_fit_per_generation.size(), 37u);
}

    // the last recorded best matches the best individual returned

TEST(RunGA, LastRecordedBestMatchesReturnedBest) {
    auto result = run_on("map_08.csv", 30, 50, 0.1f, 42);

    ASSERT_FALSE(result.best_fit_per_generation.empty());
    EXPECT_EQ(result.best_fit_per_generation.back(), result.best.get_fitness());
}

    // the best individual is a valid tour of the map

TEST(RunGA, BestIsValidTour) {
    auto result = run_on("map_08.csv", 30, 50, 0.1f, 42);

    EXPECT_TRUE(is_valid_permutation(result.best.get_genome(), 8));
}

    // the best individual's fitness is its real tour length

TEST(RunGA, BestFitnessMatchesTourLength) {
    Matrix m = load_map("map_08.csv");
    auto result = run_on("map_08.csv", 30, 50, 0.1f, 42);

    EXPECT_EQ(result.best.get_fitness(), tour_length(m, result.best.get_genome()));
}

    // the same seed and settings give the same result

TEST(RunGA, SameSeedGivesSameResult) {
    auto a = run_on("map_12.csv", 30, 50, 0.1f, 7);
    auto b = run_on("map_12.csv", 30, 50, 0.1f, 7);

    EXPECT_EQ(a.best.get_genome(), b.best.get_genome());
    EXPECT_EQ(a.best_fit_per_generation, b.best_fit_per_generation);
}

    // a mutation rate of 0 still runs, and elitism still keeps the best

TEST(RunGA, WorksWithZeroMutationRate) {
    auto result = run_on("map_08.csv", 30, 50, 0.0f, 42);

    ASSERT_EQ(result.best_fit_per_generation.size(), 50u);
    EXPECT_TRUE(is_valid_permutation(result.best.get_genome(), 8));
    for (size_t gen = 1; gen < result.best_fit_per_generation.size(); gen++){
        EXPECT_LE(result.best_fit_per_generation[gen], result.best_fit_per_generation[gen - 1]);
    }
}

    // finds the known optimal tour on the example maps (see maps/README.md)

TEST(RunGA, FindsOptimumOnMap05) {
    for (unsigned seed = 1; seed <= 5; seed++){
        auto result = run_on("map_05.csv", 30, 100, 0.1f, seed);
        EXPECT_EQ(result.best.get_fitness(), 280) << "seed " << seed;
    }
}

TEST(RunGA, FindsOptimumOnMap08) {
    for (unsigned seed = 1; seed <= 3; seed++){
        auto result = run_on("map_08.csv", 100, 300, 0.1f, seed);
        EXPECT_EQ(result.best.get_fitness(), 282) << "seed " << seed;
    }
}

    // map_20 has unconnected edges but always has a valid tour, so the GA should find one

TEST(RunGA, AvoidsUnconnectedEdgesOnMap20) {
    auto result = run_on("map_20.csv", 100, 300, 0.1f, 42);

    EXPECT_LT(result.best.get_fitness(), Individual::m_PENALTY)
        << "best tour still uses an unconnected edge";
}

//! write_progress tests -------------------------------------------------------------------

    // writes one exact JSON line for a known individual (locks down the format the worker reads)

TEST(WriteProgress, WritesExactLineForKnownIndividual) {
    std::ostringstream out;

    Genetic_algorithm::write_progress(out, 1, with_genome(MAP, MIDDLE));   // MIDDLE scores 45

    EXPECT_EQ(out.str(), "{ \"Gen\" : 1 , \"Fitness\" : 45 , \"Genome\" : [0,1,2,3] }\n");
}

    // writes exactly one line per call

TEST(WriteProgress, WritesOneLinePerCall) {
    std::ostringstream out;

    Genetic_algorithm::write_progress(out, 1, with_genome(MAP, BEST));
    Genetic_algorithm::write_progress(out, 2, with_genome(MAP, WORST));

    EXPECT_EQ(lines_of(out.str()).size(), 2u);
}

    // run_ga writes exactly one progress line per generation

TEST(WriteProgress, RunGAWritesOneLinePerGeneration) {
    std::ostringstream out;

    run_on("map_05.csv", 20, 25, 0.1f, 42, &out);

    EXPECT_EQ(lines_of(out.str()).size(), 25u);
}

    // every line run_ga writes is a complete JSON object, numbered 1, 2, 3 ...,
    // and reports the same best fitness as the returned history

TEST(WriteProgress, RunGALinesMatchHistory) {
    std::ostringstream out;

    auto result = run_on("map_08.csv", 30, 20, 0.1f, 42, &out);
    auto lines = lines_of(out.str());

    ASSERT_EQ(lines.size(), result.best_fit_per_generation.size());
    for (size_t g = 0; g < lines.size(); g++){
        const std::string& line = lines[g];
        std::string expected_gen     = "{ \"Gen\" : " + std::to_string(g + 1) + " , ";
        std::string expected_fitness = "\"Fitness\" : " + std::to_string(result.best_fit_per_generation[g]) + " , ";

        EXPECT_EQ(line.rfind(expected_gen, 0), 0u)                << "line " << g + 1 << ": " << line;   // starts with
        EXPECT_NE(line.find(expected_fitness), std::string::npos) << "line " << g + 1 << ": " << line;
        EXPECT_EQ(line.back(), '}')                               << "line " << g + 1 << ": " << line;   // ends with
    }
}
