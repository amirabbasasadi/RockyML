#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <rocky/linear.h>


TEST_CASE("Linear Layer (double precision, no bias)", "[linear][double]") {
    using namespace rocky;
    const unsigned N_in = 64;
    const unsigned B_in = 16;
    const unsigned N_out = 8;

    layer::linear<double, B_in, N_in, N_out, layer::opt::no_bias> l1; 
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
        l1.feed(X_layer, X_in, X_out);     
        return;
    };
    

    delete[] X_in;
    delete[] X_layer;
    delete[] X_out;
}

TEST_CASE("Linear Layer (float precision, no bias)", "[linear][float]") {
    using namespace rocky;
    const unsigned N_in = 64;
    const unsigned B_in = 16;
    const unsigned N_out = 8;

    layer::linear<float, B_in, N_in, N_out, layer::opt::no_bias> l1; 
    REQUIRE ( l1.deduce_num_params() == N_in*N_out );
    std::random_device rd;
    std::mt19937 rnd_gen(rd());
    std::uniform_real_distribution<float> dist(-1.0, 1.0);
    auto sampler = std::bind(dist, rnd_gen);
        
    float* X_in = new float[B_in * N_in];
    float* X_layer = new float[l1.deduce_num_params()];
    float* X_out = new float[B_in * N_out];

    std::generate(X_in, X_in + B_in * N_in, sampler);
    std::generate(X_layer, X_layer + l1.deduce_num_params(), sampler);
    
    BENCHMARK("Forward Pass") {
        l1.feed(X_layer, X_in, X_out);     
        return;
    };
    

    delete[] X_in;
    delete[] X_layer;
    delete[] X_out;
}


TEST_CASE("Linear Layer (double precision, bias)", "[linear][double]") {
    using namespace rocky;
    const unsigned N_in = 64;
    const unsigned B_in = 16;
    const unsigned N_out = 8;

    layer::linear<double, B_in, N_in, N_out, layer::opt::bias> l1; 
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
        l1.feed(X_layer, X_in, X_out);     
        return;
    };
    

    delete[] X_in;
    delete[] X_layer;
    delete[] X_out;
}

TEST_CASE("MLP Layer (double precision, bias)", "[mlp][double]") {
    using namespace rocky;
    const unsigned in_dim = 64;
    const unsigned out_dim = 16;
    const unsigned hidden_dim = 8;
    const unsigned in_num = 16;
    const unsigned layers_num = 32;

    layer::mlp<double, layers_num, in_num, in_dim, out_dim, hidden_dim, layer::opt::bias> net; 
    
    std::random_device rd;
    std::mt19937 rnd_gen(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    auto sampler = std::bind(dist, rnd_gen);
        
    
    BENCHMARK("Forward Pass") {
        double* X_in = new double[in_num * in_dim];
        double* X_net = new double[net.deduce_num_params()];
        double* X_out = new double[in_num * out_dim];

        std::generate(X_in, X_in + in_num * in_dim, sampler);
        std::generate(X_net, X_net + net.deduce_num_params(), sampler);
        
        delete[] X_in;
        delete[] X_net;
        delete[] X_out;

        return;
    };
}

TEST_CASE("MLP Layer (single precision, bias)", "[mlp][float]") {
    using namespace rocky;
    const unsigned in_dim = 64;
    const unsigned out_dim = 16;
    const unsigned hidden_dim = 8;
    const unsigned in_num = 16;
    const unsigned layers_num = 32;

    layer::mlp<float, layers_num, in_num, in_dim, out_dim, hidden_dim, layer::opt::bias> net; 
    
    std::random_device rd;
    std::mt19937 rnd_gen(rd());
    std::uniform_real_distribution<float> dist(-1.0, 1.0);
    auto sampler = std::bind(dist, rnd_gen);
        
    
    BENCHMARK("Forward Pass") {
        float* X_in = new float[in_num * in_dim];
        float* X_net = new float[net.deduce_num_params()];
        float* X_out = new float[in_num * out_dim];

        std::generate(X_in, X_in + in_num * in_dim, sampler);
        std::generate(X_net, X_net + net.deduce_num_params(), sampler);
        
        delete[] X_in;
        delete[] X_net;
        delete[] X_out;

        return;
    };   
}