#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <rocky/zagros/containers/swarm.h>

TEST_CASE("Creating a swarm container", "[swarm]"){
    using namespace rocky;

    typedef double swarm_type;

    const int n_particles = 100;
    const int group_size = 20;
    const int dim = 16;

    zagros::basic_swarm<swarm_type, dim> swarm(n_particles, group_size);

    auto rng = swarm.group_range(0);

    REQUIRE(rng.first == 0);
    REQUIRE(rng.second == group_size);

    swarm.allocate();

    REQUIRE(swarm.particle(0) != nullptr);
    REQUIRE(swarm.group(0) != nullptr);

    std::fill(swarm.particle(0), swarm.particle(0)+dim, 3.14);

    REQUIRE(*(swarm.particle(0) + 2) == 3.14);
    
    REQUIRE(*(swarm.value(5)) > 1000.0);
    
};