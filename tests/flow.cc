#define ROCKY_USE_MPI
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>


TEST_CASE("Creating a flow", "[flow][zagros][rocky]"){
    using namespace rocky;

    typedef double swarm_type;

    const int n_particles = 100;
    const int group_size = 10;
    const int dim = 1000;
    const int block_dim = 500;

    using namespace zagros::dena;
    

    zagros::benchmark::rastrigin<swarm_type> problem(dim);

    auto f1 = container::create("A", n_particles, group_size)
              >> pso::memory::create("M", "A")
              >> init::uniform("A")
              >> run::n_times(20, pso::group::step("M", "A"));
    
    zagros::basic_runtime<swarm_type, dim, block_dim> runtime(&problem);
    runtime.run(f1);

};