#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <rocky/zagros/strategy.h>
#include <rocky/zagros/benchmark.h>


TEST_CASE("Gaussian mutation strategy", "[strategy][double][benchmark]"){
    using namespace rocky;

    const int dim = 15;
    const int n_pop = 100;
    const int n_tribes = 5;
    const int tribe_size = n_pop / n_tribes;
    const int max_affected_dim = 3;

    // create a random population
    auto system = new zagros::benchmark::sphere<double, dim>();
    double* population = new double[n_pop * dim];
    double* values = new double[n_pop * dim];
    // evalutae each particle
    tbb::parallel_for(0, n_pop, 1, [&](int p){
        values[p] = system->objective(population + p*dim);
    });

    auto strategy = zagros::gaussian_mutation<double, dim, tribe_size, max_affected_dim>();

    BENCHMARK("apply gaussian mutation"){
        strategy.apply(system, population, values);
    };
    
    delete system;
    delete[] values;
    delete[] population;
}