#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <rocky/zagros/flow.h>

TEST_CASE("Creating a flow", "[flow][zagros][rocky]"){
    using namespace rocky;

    typedef double swarm_type;

    const int n_particles = 100;
    const int group_size = 20;
    const int dim = 16;
    int max_strategies = 2;

    zagros::basic_flow<swarm_type, dim> flow;
    flow.set_state(nullptr);
    REQUIRE(flow.state() == nullptr);

};