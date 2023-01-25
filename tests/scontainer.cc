#define ROCKY_USE_MPI
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <list>
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>


TEST_CASE("solution container operations", "[container][zagros][rocky]"){

    using namespace rocky;

    typedef double solution_type;

    const int n_particles = 100;
    const int group_size = 10;
    const int dim = 1000;

    using namespace zagros::dena;
    
    zagros::basic_scontainer<solution_type, dim> container(n_particles, group_size);
    container.allocate();


    REQUIRE(container.space() == sizeof(solution_type) * (dim+1) * n_particles);
    REQUIRE(container.n_particles() == n_particles);
    REQUIRE(container.n_groups() == n_particles / group_size);

    SECTION("find top-k solutions"){
        container.values[10] = 10.5;
        container.values[75] = 4.5;
        container.values[7] = 3.5;
        int top[3];
        container.best_k(top, 3);
        REQUIRE(top[0] == 7);
        REQUIRE(top[1] == 75);
        REQUIRE(top[2] == 10);       
    }
    SECTION("find worst k solutions"){
        std::fill(container.values.begin(), container.values.end(), 0.0);
        int worst[2];
        container.values[50] = 10.0;
        container.values[17] = 8.0;

        container.worst_k(worst, 2);
        REQUIRE(worst[0] == 50);
        REQUIRE(worst[1] == 17);  
    }
    SECTION("selecting best solution from another container"){
        zagros::basic_scontainer<solution_type, dim> c2(3, 3);
        c2.allocate();
        c2.values[0] = -1.0;
        c2.values[1] = 1.0;
        c2.values[2] = -1.5;

        container.replace_with(&c2);
        int top[3];
        container.best_k(top, 3);
        REQUIRE(container.values[top[0]] == c2.values[2]);
        REQUIRE(container.values[top[1]] == c2.values[0]);
        REQUIRE(container.values[top[2]] == c2.values[1]);  
    }
    SECTION("sampling particles"){
        const int n = 2;
        int samples[n];
        container.sample_n_particles(samples, n);
        for(int i=0; i<n; i++)
            REQUIRE(samples[i] > 0);
        REQUIRE(samples[0] != samples[1]);
    }

};