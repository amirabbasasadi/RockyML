#define ROCKY_USE_MPI
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <list>
#include <rocky/zagros/containers/scontainer.h>
#include <rocky/zagros/strategies/init.h>
#include <rocky/zagros/strategies/genetic.h>
#include <rocky/zagros/strategies/eda.h>
#include <rocky/zagros/strategies/differential_evolution.h>
#include <rocky/zagros/benchmark.h>


TEST_CASE("strategy", "[strategy][zagros][rocky]"){

    using namespace rocky;

    typedef double container_type;

    const int n_particles = 100;
    const int group_size = 10;
    const int dim = 1000;

    
    zagros::benchmark::rastrigin<container_type> problem(dim, 1.0);


    zagros::basic_scontainer<container_type, dim> container(n_particles, group_size);
    container.allocate();

    zagros::basic_scontainer<container_type, dim> candidates(n_particles, group_size);
    candidates.allocate();
    
    
    zagros::uniform_init_strategy<container_type, dim> init_str(&problem, &container);

    init_str.apply();
    container.evaluate_and_update(&problem);

    SECTION("gaussian mutation"){
        int affected_dims = 8;
        zagros::gaussian_mutation<container_type, dim> str(&problem, &container, &candidates, affected_dims, 1.0, 0.5);
        BENCHMARK("gaussian mutation"){
            str.apply();
        };
    };

    SECTION("multipoint crossover"){
        int affected_dims = 8;
        zagros::multipoint_crossover<container_type, dim> str(&problem, &container, &candidates, affected_dims);
        BENCHMARK("multipoint crossover"){
            str.apply();
        };
    };
    
    SECTION("segment crossover"){
        int segment_len = 8;
        zagros::static_segment_crossover<container_type, dim> str(&problem, &container, &candidates, segment_len);
        BENCHMARK("segment crossover"){
            str.apply();
        };
    };

    SECTION("differential evolution"){
        zagros::basic_differential_evolution<container_type, dim> str(&problem, &container, &candidates);
        BENCHMARK("differential evolution"){
            str.apply();
        };
    };

    SECTION("estimation of distribution (MVN)"){
        int samples = 100;
        zagros::eda_mutivariate_normal<container_type, dim> str(&problem, &container, &candidates, samples);
        BENCHMARK("estimation of distribution (MVN)"){
            str.apply();
        };
    };

};