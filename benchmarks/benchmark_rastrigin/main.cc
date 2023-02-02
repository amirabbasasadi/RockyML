#define ROCKY_USE_MPI
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>
#include <rocky/zagros/strategies/eda.h>

#include<chrono>

using namespace rocky;
using namespace zagros::dena;
using namespace std::chrono;
    
flow get_optimizer(int n_particles){
    return container::create("A", n_particles)
              >> init::uniform("A") 
              >> run::n_times(500,
                        mutate::gaussian("A")
                        >> run::with_probability(0.2,
                            crossover::differential_evolution("A")
                        )
              );
}

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    typedef float solution_type;

    const int dim = 100;
    const int block_dim = 100;

    zagros::benchmark::rastrigin<solution_type> problem(dim);

    auto n_threads = argv[1];

    std::fstream fh("../benchmark.data", std::fstream::app);

    std::vector<int> particles {100, 200, 400, 600, 800, 1000, 2000, 4000, 6000, 8000, 10000};
    
    for(int i=0; i<particles.size(); i++){
        auto f = get_optimizer(particles[i]);
        auto start = high_resolution_clock::now();
        zagros::basic_runtime<solution_type, dim, block_dim> runtime(&problem);
        runtime.run(f);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        fh << fmt::format("{},{},{}", n_threads, particles[i], duration.count()) << std::endl;
    }

    fh.close();
 
    MPI_Finalize();
    return 0;
}

