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

    auto f2 = container::create("A", n_particles, group_size)
              >> pso::memory::create("M", "A")
              >> init::uniform("A")
              >> run::n_times(60,
                    blocked_descent::uniform::step()
                    >> run::n_times(50, 
                            pso::group::step("M", "A"))
                    >> run::n_times(120, 
                            pso::cluster::step("M", "A")
                            >> log::local::best("loss.csv")));

    zagros::basic_runtime<swarm_type, dim, block_dim> runtime(&problem);
    runtime.run(f2);
    MPI_Finalize();
    return 0;
}

