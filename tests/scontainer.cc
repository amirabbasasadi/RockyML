#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <rocky/zagros/containers/scontainer.h>

TEST_CASE("Creating a solution container", "[scontainer]"){
    using namespace rocky;

    typedef double container_type;

    const int n_particles = 100;
    const int group_size = 20;
    const int dim = 16;

    zagros::basic_scontainer<container_type, dim> container(n_particles, group_size);

    auto rng = container.group_range(0);

    REQUIRE(rng.first == 0);
    REQUIRE(rng.second == group_size);

    container.allocate();

    REQUIRE(container.particle(0) != nullptr);
    REQUIRE(container.group(0) != nullptr);

    std::fill(container.particle(0), container.particle(0)+dim, 3.14);

    REQUIRE(*(container.particle(0) + 2) == 3.14);
    
    REQUIRE(*(container.value(5)) > 1000.0);
    
};