#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <rocky/linear.h>


TEST_CASE("Linear Layer (double precision, no bias)", "[benchmark]") {
    using namespace rocky;
    const unsigned N_in = 256;
    const unsigned B_in = 2048;
    const unsigned N_out = 128;

    layer::linear<double, N_in, N_out, layer::opt::no_bias> l1; 
    REQUIRE ( l1.deduce_num_params() == N_in*N_out );
    std::random_device rd;
    std::mt19937 rnd_gen(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    auto sampler = std::bind(dist, rnd_gen);
        
    double* X_in = new double[B_in * N_in];
    double* X_layer = new double[l1.deduce_num_params()];
    double* X_out = new double[B_in * N_out];

    std::generate(X_in, X_in + B_in * N_in, sampler);
    std::generate(X_layer, X_layer + l1.deduce_num_params(), sampler);
    
    BENCHMARK("Forward Pass") {
        l1.apply(X_layer, X_in, B_in, X_out);     
        return;
    };
    

    delete[] X_in;
    delete[] X_layer;
    delete[] X_out;
}

TEST_CASE("Linear Layer (double precision, biased)", "[benchmark]") {
    using namespace rocky;
    const unsigned N_in = 256;
    const unsigned B_in = 2048;
    const unsigned N_out = 128;

    layer::linear<double, N_in, N_out> l1; 
    REQUIRE ( l1.deduce_num_params() == (N_in+1)*N_out );
    std::random_device rd;
    std::mt19937 rnd_gen(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    auto sampler = std::bind(dist, rnd_gen);
        
    double* X_in = new double[B_in * N_in];
    double* X_layer = new double[l1.deduce_num_params()];
    double* X_out = new double[B_in * N_out];

    std::generate(X_in, X_in + B_in * N_in, sampler);
    std::generate(X_layer, X_layer + l1.deduce_num_params(), sampler);
    
    BENCHMARK("Forward Pass") {
        l1.apply(X_layer, X_in, B_in, X_out);     
        return;
    };
    

    delete[] X_in;
    delete[] X_layer;
    delete[] X_out;
}