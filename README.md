<p><img align="center" src="/logo/rockyml-poster-white-2500.png"></p>
<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blueviolet?style=flat">
  <a href="https://amirabbasasadi.github.io/RockyML" target="_blank"><img src="https://img.shields.io/badge/view-Documentation-red?"></a>
  <img src="http://img.shields.io/github/actions/workflow/status/amirabbasasadi/RockyML/ctest.yml?branch=main">
  <img src="https://img.shields.io/github/commit-activity/m/amirabbasasadi/RockyML">
  <img src="https://img.shields.io/github/license/amirabbasasadi/RockyML">
</p>  
<br>
<br>
<p align="center">📔 Documentation : <a target="_blank" href="https://amirabbasasadi.github.io/RockyML">amirabbasasadi.github.io/RockyML</a> </p>

## Tutorials

### Zagros Tutorials
- [Using with CMake](https://amirabbasasadi.github.io/RockyML/build.html)
- [Create your first optimizer with Dena and Zagros](https://amirabbasasadi.github.io/RockyML/zagros_minimal.html)
- [Define a custom optimization problem in Zagros](https://amirabbasasadi.github.io/RockyML/zagros_system.html)
- [Block optimization for large problems](https://amirabbasasadi.github.io/RockyML/block_optimization.html)


## Components
### Zagros
Design Goals:
- Providing a language called Dena for designing arbitrary complex optimizers by combining 
  - Modular and parallel search strategies: Genetic, PSO, EDA, ...
  - Communication strategies for distributed optimization on top of MPI
  - Analyzer strategies for analyzing objective functions
  - Blocking strategies for block optimization
  - Logging strategies for tracking optimization experiments on local system or a remote server
- Hybrid parallelism: multi-threading in each node and message passing across nodes (MPI)
- ‌Block optimization for using memory-intensive optimizers for large number of variables

```cpp
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
```


### Etna (Work in progress)
Building blocks for designing non-differentiable neural networks

- Fast, low overhead, and thread-safe 
- Various components:
  - Standard deep learning layers
  - Discrete and integer layers
  - Combinatorial layers
  - Stochastic layers
  - Dynamic layers
