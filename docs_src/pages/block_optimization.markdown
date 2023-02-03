# Block optimization for large problems{#block_optimization}
Many search strategies in Zagros are population-based methods that are memory intensive. This can be problematic for large-scale problems. support we want to optimizer a function with 1 million variables. If our memory is limited, It's definitely not a good idea to use large solution containers. To address this, Zagros supports block optimization. The idea is simple. Instead of optimizing all variables, we select a subset of variable and optimize them, actually some kind of coordinate descent. We optimize and then again select another group of variables and keep doing this until convergence. But how to select those variables? Dena blocking strategies can specify the method for variable selection. 
## Example
```cpp
const int dim = 100000;
const int block_dim = 1000;
// problem definition
zagros::benchmark::ackley<float> problem(dim);

auto optimizer = container::creat("A", 1000)
                 >> init::uniform("A")
                 run::while_improve(5,
                    block::uniform::select() 
                    >> run::while_improve(5
                        eda::mvn::full_cov("A")
                    )
                 );

// specify the block size when defining the runtime
zagros::basic_runtime<float, dim, block_dim> runtime(&problem);
```
Now in each step we are solving a 1000-dimensional problem which consumes much less memory.