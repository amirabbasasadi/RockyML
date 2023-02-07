# Distributed optimization, Basic example{#zagros_distributed}
This is a basic example of using Zagros and Dena for distributed optimization. Currently Zagros uses MPI as the backend for communication so MPI should be properly initialized in your code:

```cpp
#include <mpi.h>
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>

using namespace rocky::zagros;
using namespace rocky::zagros::dena;

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    

    MPI_Finalize();
    return 0;
}
```
Now suppose we have the following optimizer in Dena:
```cpp
auto optimizer = container::create("A", 1000)
                >> init::uniform("A") 
                >> run::n_times(500,
                    eda::mvn::full_cov("A")
                    >> crossover::segment("A", 4)
                );
```
This optimizer is a simple mixture of EDA and Genetic crossovers. After defining the problem and runtime you can simply execute it using MPI:
```cpp
// define the optimization problem
const int dim = 100;
benchmark::rastrigin<float> problem(dim);

// create a runtime for executing the optimizer 
basic_runtime<float, dim> runtime(&problem);
runtime.run(optimizer);
```
And the execution:
```
mpirun -np 1 ./app
```
The above command will execute the optimizer with a single process. for execution with 2 processes we can just change the `-np` parameter:
```
mpirun -np 2 ./app
```
Also don't forget to assign the proper resources to your processes. for example maybe you like to assign 2 CPU threads to each process:
```
mpirun --use-hwthread-cpus -np 2 --map-by socket:PE=2 ./app
```
Read more about [process affinity](https://www.ibm.com/docs/en/smpi/10.2?topic=administering-managing-process-placement-affinity) in the documentation of your MPI implementation. 
## Solution propagation
So far if we execute the app, two processes will run simultaniously, however they are independent and don't exchange any information. Dena provides utilities for sharing the information between optimizer so one optimizer can inform the other nodes in the cluster. Let's create another solution container called `best` for holding the best solution in a node:
```cpp
auto optimizer = container::create("A", 1000)
                    >> container::create("best")
                    >> init::uniform("A") 
                    >> run::n_times(500,
                        eda::mvn::full_cov("A")
                        >> crossover::segment("A", 4)
                    );
```
To update the `best` we can use `container::take_best(dest, src)` syntax. This command takes the name of two containers as input namely a "destination" container and a "source". if the best solution in "source" is better than the worst solution in "destination", the solution in "source" will be replaced by that solution. Thus to ensure it contains the best solution we can take the best solutions from `A` at each step:
```cpp
auto optimizer = container::create("A", 1000)
                    >> container::create("best")
                    >> init::uniform("A") 
                    >> run::n_times(500,
                        eda::mvn::full_cov("A")
                        >> crossover::segment("A", 4)
                        >> container::take_best("best", "A")
                    );
```
Now we are sure that `best` holds the best solution in this process. Next we are going to ask all processes in the cluster, who has the best value for this container? whoever is it, should broadcast it for everyone else. To do so we can simply use `propagate::cluster::best()` like this:
```cpp
propagate::cluster::best("best")
```
However note this is a collective call performing a synchronization for all processes, so it's better not to call it frequently:
```cpp
run::ever_n_steps(100,
    propagate::cluster::best("best")
)
```
Now we need to make sure the  retreived solution will be available in `A` so the mutation and EDA algorithms can use it.
```cpp
run::ever_n_steps(100,
    propagate::cluster::best("best")
    >> container::take_best("A", "best")
)
```
## Tracking the result using Comet
Suppose you have many nodes and you want to track the best found solution in all of them. In addition to local logging, Dena supports logging the result on [Comet](https://comet.ml/). First you need to create a `comet_log_handler` object:
```cpp
comet_log_handler log_handler("YOUR_COMET_API_KEY", "workspace", "project", "best_solution");
```
Now using `log::comet::best(cnt, log_handler)`, we can log the best solution in container `cnt` at each step. Do the possible latency for sending the result to Comet server, we can perform this operation with a certain probability:
```cpp
run::with_probability(0.1,
    log::comet::best("best", log_handler)
)
```
Now the result be available in your Comet dashbord.
## Conclusion
In this tutorial we saw a basic example of designing a distributed optimizer:
```cpp
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
```
