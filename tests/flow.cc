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

    using namespace zagros::dena;
    

    zagros::benchmark::rastrigin<swarm_type, dim> problem;

    auto f2 = container::create("S", n_particles, group_size)
              >> pso::memory::create("mem", "S")
              >> init::uniform("S")
              >> pso::group_level::step("mem", "S");
    
    zagros::basic_runtime<swarm_type, dim> runtime(&problem);
    runtime.run(f2);
    spdlog::info("runtime storage : {} MB", runtime.storage.container_space()/(1024.0*1024.0));
};