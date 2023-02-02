#include <mpi.h>
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>

using namespace rocky::zagros;
using namespace rocky::zagros::dena;

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    
    // define the optimization problem
    const int dim = 100;
    benchmark::rastrigin<float> problem(dim);

    // recording the result of optimization
    local_log_handler log_handler("result.csv");

    // define the optimizer
    auto optimizer = container::create("A", 300)
                    >> init::uniform("A") 
                    >> run::n_times(500,
                            mutate::gaussian("A")
                            >> run::with_probability(0.2,
                                crossover::differential_evolution("A")
                            )
                            >> log::local::best("A", log_handler)
                        );

    // create a runtime for executing the optimizer 
    basic_runtime<float, dim> runtime(&problem);
    runtime.run(optimizer);

    MPI_Finalize();
    return 0;
}