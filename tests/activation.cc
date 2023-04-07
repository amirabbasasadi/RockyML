#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <rocky/etna/blocks.h>


TEST_CASE("Activations (double precision)", "[etna][double]") {
    using namespace rocky;
    const unsigned N_in = 64;

    std::random_device rd;
    std::mt19937 rnd_gen(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    auto sampler = std::bind(dist, rnd_gen);
    
    std::vector<double> X_in(N_in);

    std::generate(X_in.begin(), X_in.end(), sampler);
    
    etna::act::step<double> step_fn;

    BENCHMARK("Step function") {
        step_fn(X_in.data(), N_in);   
        return;
    };

}
