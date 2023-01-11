#define ROCKY_USE_MPI
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>


int main(int argc, char* argv[]){
    using namespace rocky;
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    using namespace rocky;

    typedef float swarm_type;

    const int n_particles = 50;
    const int group_size = 10;
    const int dim = 100;
    const int block_dim = 50;

    using namespace zagros::dena;
    

    zagros::benchmark::rastrigin<swarm_type> problem(dim);

    auto f2 = container::create("A", n_particles, group_size)
              >> pso::memory::create("M", "A")
              >> init::uniform("A")
              >> run::n_times(100, pso::group_level::step("M", "A")
                                   >> run::with_probability(1.0, 
                                           log::local::best(pso::memory::particles_mem("M"), "loss_track_1.data")
                                    ));
    
    zagros::basic_runtime<swarm_type, dim, block_dim> runtime(&problem);
    runtime.run(f2);
    spdlog::info("runtime storage : {} MB", runtime.storage.container_space()/(1024.0*1024.0));
    MPI_Finalize();
    return 0;
}

