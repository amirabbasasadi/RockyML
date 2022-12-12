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
    flow.allocate(max_strategies);

    SECTION("stochastic transitions"){
        flow.set_transition(0, 0, 0.5);
        flow.set_transition(0, 1, 0.5);
        flow.set_transition(1, 0, 0.5);
        flow.set_transition(1, 1, 0.5);
        flow.set_state(0);
        REQUIRE(flow.state() == 0);
    }

    SECTION("deterministic transitions"){
        flow.set_transition(0, 0, 0.0);
        flow.set_transition(0, 1, 1.0);
        flow.set_transition(1, 0, 0.0);
        flow.set_transition(1, 1, 1.0);
        flow.set_state(0);
        REQUIRE(flow.state() == 0);
        flow.step();
        REQUIRE(flow.state() == 1);
        flow.step();
        REQUIRE(flow.state() == 1);
    }

};