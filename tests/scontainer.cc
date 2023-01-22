#define ROCKY_USE_MPI
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <list>
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>


TEST_CASE("Creating a container", "[container][zagros][rocky]"){

    using namespace rocky;

    typedef double solution_type;

    const int n_particles = 100;
    const int group_size = 10;
    const int dim = 1000;

    using namespace zagros::dena;
    
    zagros::basic_scontainer<solution_type, dim> container(n_particles, group_size);

    REQUIRE(container.n_particles() == n_particles);

};