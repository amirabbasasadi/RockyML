#define ROCKY_USE_MPI
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
    const int group_size = 10;
    const int dim = 100;
    const int block_dim = 10;

    using namespace zagros::dena;
    

    zagros::benchmark::rastrigin<swarm_type> problem(dim);

    auto f2 = container::create("A", n_particles, group_size)
              >> pso::memory::create("M", "A")
              >> init::uniform("A")
              >> run::n_times(10, pso::group_level::step("M", "A")
                                   >> run::with_probability(0.1, 
                                           log::local::best(pso::memory::particles_mem("M"), "loss_track_1.data")
                                      ));
    
    zagros::basic_runtime<swarm_type, dim, block_dim> runtime(&problem);
    runtime.run(f2);
    spdlog::info("runtime storage : {} MB", runtime.storage.container_space()/(1024.0*1024.0));
};