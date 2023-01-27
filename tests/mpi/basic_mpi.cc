#define ROCKY_USE_MPI
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>
#include <rocky/zagros/strategies/eda.h>


int main(int argc, char* argv[]){
    using namespace rocky;
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    using namespace rocky;

    typedef float swarm_type;

    const int n_particles = 300;
    const int group_size = 10;
    const int dim = 200;
    const int block_dim = 20;

    using namespace zagros::dena;
    

    zagros::benchmark::rastrigin<swarm_type> problem(dim);
    std::fstream f;
    zagros::local_optimization_log log_handler(f, "loss.csv");
    
    auto f2 = container::create("A", n_particles, group_size)
              >> pso::memory::create("M", "A")
              >> init::uniform("A")
              >> run::while_improve(10,
                    blocked_descent::uniform::step()
                    >> run::while_improve(5, 
                            pso::local::step("M", "A")
                            >> mutate::gaussian(pso::memory::particles_mem("M"), 4)
                            >> crossover::multipoint(pso::memory::particles_mem("M"))
                            >> run::with_probability(0.05, log::local::best(log_handler)))
                    >> run::while_improve(5, 
                            pso::global::step("M", "A")
                            >> run::with_probability(0.05, log::local::best(log_handler))));

    zagros::basic_runtime<swarm_type, dim, block_dim> runtime(&problem);
    
    runtime.run(f2);
    MPI_Finalize();
    return 0;
}

