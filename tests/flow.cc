#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <list>
#include <rocky/zagros/flow.h>

TEST_CASE("Creating a flow", "[flow][zagros][rocky]"){
    using namespace rocky;

    typedef double swarm_type;

    const int n_particles = 100;
    const int group_size = 20;
    const int dim = 16;
    int max_strategies = 2;

    using namespace zagros::flow;

    auto f2 = container::create("c1", n_particles, group_size) >> init::uniform() >> init::uniform();

    
    zagros::basic_runtime<swarm_type, dim> runtime;
    runtime.run(f2);
};