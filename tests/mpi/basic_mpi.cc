#define ROCKY_USE_MPI
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>

using namespace rocky;
using namespace zagros::dena;
    
int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    typedef float solution_type;

    const int n_particles = 300;
    const int group_size = 20;
    const int dim = 1000;
    const int block_dim = 100;

    zagros::benchmark::rastrigin<solution_type> problem(dim);
    
    zagros::local_log_handler log_handler("best_solution.csv");

    auto f1 = container::create("A", n_particles, group_size)
              >> pso::memory::create("M", "A")
              >> init::uniform("A")
              >> run::while_improve(300,
                    block::uniform::select()
                    >> run::while_improve(10, 
                            pso::local::step("M", "A")
                            >> mutate::gaussian(pso::memory::particles_mem("M"))
                            >> run::with_probability(0.05, log::local::best(log_handler)))
                    >> run::while_improve(10, 
                            pso::global::step("M", "A")
                            >> run::with_probability(0.4, crossover::differential_evolution(pso::memory::particles_mem("M")))
                            >> run::with_probability(0.05, log::local::best(log_handler))));

    zagros::basic_runtime<solution_type, dim, block_dim> runtime(&problem);
 
    runtime.run(f1);

    MPI_Finalize();
    return 0;
}

