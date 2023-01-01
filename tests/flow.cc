#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <list>
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>


TEST_CASE("Creating a flow", "[flow][zagros][rocky]"){
    using namespace rocky;

    typedef double swarm_type;

    const int n_particles = 100;
    const int group_size = 20;
    const int dim = 16;

    using namespace zagros::dena;
    

    zagros::benchmark::rastrigin<swarm_type, dim> problem;

    auto f2 = container::create("A", n_particles, group_size)
              >> pso::memory::create("M", "A")
              >> init::uniform("A")
              >> run::n_times(250, pso::group_level::step("M", "A")
                                   >> run::with_probability(0.1, 
                                           log::local::best(pso::memory::particles_mem("M"), "loss_track.data")
                                      ));
    
    zagros::basic_runtime<swarm_type, dim> runtime(&problem);
    runtime.run(f2);
    spdlog::info("runtime storage : {} MB", runtime.storage.container_space()/(1024.0*1024.0));
};