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

    const int n_particles = 300;
    const int group_size = 10;
    const int dim = 2000;
    const int block_dim = 100;

    using namespace zagros::dena;
    

    zagros::benchmark::rastrigin<swarm_type> problem(dim);
    std::fstream f;
    zagros::local_optimization_log log_handler(f, "loss.csv");

    auto f2 = container::create("A", n_particles, group_size)
              >> pso::memory::create("M", "A")
              >> init::uniform("A")
              >> run::n_times(10,
                    blocked_descent::uniform::step()
                    >> run::n_times(20, 
                            pso::group::step("M", "A"))
                    >> run::every_n_steps(5, 
                            pso::cluster::step("M", "A")
                            >> run::with_probability(1.0, log::local::best(log_handler))));

    zagros::basic_runtime<swarm_type, dim, block_dim> runtime(&problem);
    runtime.run(f2);
    MPI_Finalize();
    return 0;
}

