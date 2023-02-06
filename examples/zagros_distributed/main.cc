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
    comet_log_handler log_handler("Comet_API_KEY", "workspace", "project", "best_solution");

    // define the optimizer
    auto optimizer = container::create("A", 1000)
                    >> container::create("best")
                    >> init::uniform("A") 
                    >> run::n_times(500,
                            eda::mvn::full_cov("A")
                            >> crossover::segment("A", 4)
                            >> container::take_best("best", "A")
                            >> run::every_n_steps(200,
                                propagate::cluster::best("best")
                                >> container::take_best("A", "best")
                            )
                            >> run::with_probability(0.1,
                                log::comet::best("best", log_handler)
                            )
                        );

    // create a runtime for executing the optimizer 
    basic_runtime<float, dim> runtime(&problem);
    runtime.run(optimizer);

    MPI_Finalize();
    return 0;
}