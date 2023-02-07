# Designing Search Algorithms with Dena{#dena}


## Flow control
<table>
  <tr>
    <th style="width:300px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`run::n_times(n, f)`</td>
    <td>Execute the flow `f` for `n` times</td>
    <td></td>
  </tr>
  <tr>
    <td>`run::with_probability(p, f)`</td>
    <td>Execute the flow `f` with probability `p`</td>
    <td></td>
  </tr>
  <tr>
    <td>`run::every_n_steps(n, f)`</td>
    <td>Execute the flow `f` every `n` steps</td>
    <td></td>
  </tr>
  <tr>
    <td>`run::while_improve(cnt, w, f)`</td>
    <td>Repeat the flow `f` if there has been any improvement in the container `cnt`. Terminates the flow execution if after waiting `w` steps observes no improvements. </td>
    <td>Also can be use like without passing a container id that is `run::while_improve(w, f)`, in this case will track the best solution in the node.</td>
  </tr>
</table>

## Container manipulation strategies
<table>
  <tr>
    <th style="width:450px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`container::create(id, n_particles, group_size)`</td>
    <td>Allocates a solution container for `n_particles` divided to groups with size of `group_size`.</td>
    <td></td>
  </tr>
  <tr>
    <td>`container::take_best(destination, source)`</td>
    <td>If there exists any solution in the `source` container that is better than a solution in the `destination`, then it will replace that solution.</td>
    <td></td>
  </tr>
  <tr>
    <td>`container::eval(id)`</td>
    <td>Evaluates all solution in a container and update the values.</td>
    <td>Dena search strategies will do this automatically. you may need this only if you have manually applied a change to a solution container</td>
  </tr>
</table>

## Initialization strategies
Used for initializing the solutions in a solution container.
<table>
  <tr>
    <th style="width:250px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`init::uniform(id)`</td>
    <td>intializes a solution container using a real uniform distribution</td>
    <td>This strategy will not evaluate the solutions after initialization.</td>
  </tr>
</table> 

## Search strategies
### Estimation of Distribution Algorithms
<table>
  <tr>
    <th style="width:250px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`eda::mvn::full_cov(cnt)`</td>
    <td>EDA using a Multivariate Normal with a full covariance matrix</td>
    <td>The size of solution container must be larger than the size of runtime's  block.</td>
  </tr>
</table> 

### Genetic Algorithms
<table>
  <tr>
    <th style="width:400px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`mutation::gaussian(cnt, d, mu, sigma)`</td>
    <td>Adds a gaussian noise to every solution in a container `cnt` and keeps the improved solutions. `d` is the number of affected dimensions. `mu` ad `sigma` are respectively the mean and std of the additive noise</td>
    <td></td>
  </tr>
  <tr>
    <td>`crossover::segment(cnt, segment_len)`</td>
    <td>Performs segment crossover and keeps the improved children. `segment_len` is length of the segment.</td>
    <td>`segment_len` should be less than the runtime block dimension</td>
  </tr>
  <tr>
    <td>`crossover::multipoint(cnt, dims)`</td>
    <td>Performs a multi-point crossover and keeps the improved children. `dims` is the number of dimensions.</td>
    <td>`dims` should be less than the runtime block dimension</td>
  </tr>
</table> 

#### References
- Simon, D., 2013. Evolutionary optimization algorithms. John Wiley & Sons.

### Differential Evolution
<table>
  <tr>
    <th style="width:450px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`crossover::differential_evolution(cnt, cr, dw)`</td>
    <td>Differential evolution on container `cnt`. `cr` is the crossover probability and `dw` is differential weight</td>
    <td>The size of the target solution container must be at least 4</td>
  </tr>
</table> 
### Particle Swarm Optimization
Dena supports a multi-level implementation of Particle Swarm Optimization, impemented based on Tribe-PSO. PSO supports container grouping so you can devide a solution container into groups. for grouping you can specify the size of groups while creating a solution container:
```cpp
// create a solution container with 100 particles divided to 5 groups
container::create("X", 100, 20)
```
Also note PSO needs a set of auxiliary solution containers to use as particles memory. These containers can be created using:
```cpp
pso::memory::create("memory_name", "target_container")
```
<table>
  <tr>
    <th style="width:350px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`pso::local::step(mem_cnt, cnt)`</td>
    <td>Performs local PSO. That is particles within a group will take one step toward the particle in the group.</td>
    <td>If you applied this on a container with a single group, it would be equivalent to the standard PSO.</td>
  </tr>
  <tr>
    <td>`pso::global::step(mem_cnt, cnt)`</td>
    <td>Performs global PSO. That is all particles will take a step toward the best solution.</td>
    <td>Using communication strategies provided by Dena, you can propagate the best found solution across cluster nodes in a distributed runtime.</td>
  </tr>
  <tr>
    <td>`pso::memory::create(mem_cnt, cnt)`</td>
    <td>Creates PSO auxiliary solution containers. `mem_cnt` is the name of created memory and `cnt` is the name of target solution container.</td>
    <td></td>
  </tr>
  <tr>
    <td>`pso::memory::particles_mem(mem_id)`</td>
    <td>Retrieve the name of the particles memory container given a mmeory id. This container contains the best observations of each particle.</td>
    <td></td>
  </tr>
  <tr>
    <td>`pso::memory::groups_mem(mem_id)`</td>
    <td>Retrieve the name of the groups memory container given a mmeory id This container contains the best observations of each group.</td>
    <td></td>
  </tr>
  <tr>
    <td>`pso::memory::node_mem(mem_id)`</td>
    <td>Retrieve the name of the node memory container given a mmeory id. This container contains the best observation in the node.</td>
    <td></td>
  </tr>
  <tr>
    <td>`pso::memory::cluster_mem(mem_id)`</td>
    <td>Retrieve the name of the cluster memory container given a mmeory id. This container contains the best observation in the cluster.</td>
    <td></td>
  </tr>
  <tr>
    <td>`pso::memory::particles_vel(mem_id)`</td>
    <td>Retrieve the name of velocity container given a mmeory id. This container contains the velocity of particles</td>
    <td></td>
  </tr>
</table> 

#### References
- Chen, K., Li, T. and Cao, T., 2006. Tribe-PSO: A novel global optimization algorithm and its application in molecular docking. Chemometrics and intelligent laboratory systems, 82(1-2), pp.248-259.

## Blocking strategies
The following strategies can be use for block optimization. It means instead of all variable only a subset of vaiables will be optimized at each step. Thus block optimization is useful for applying memory-intensive search methods on large problems. Each block strategies select the subset of variables to be optimized in a different way.  
**Note** : In a distributed runtime, the selected subset of variables (a mask) will be synchronized across all nodes so it's a collective call and can become a performance bottleneck. 
<table>
  <tr>
    <th style="width:350px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`block::uniform::select()`</td>
    <td>Randomly select a subset of variables and synchronize them acorss all nodes if the runtime is distributed</td>
    <td></td>
  </tr>
</table> 

## Communication strategies
Communication strategies are designed for distributed optimization. Currently they run on top of MPI. They can propagate the best found solutions in the cluster.
<table>
  <tr>
    <th style="width:350px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`propagate::cluster::best(id)`</td>
    <td>Find the best solution in each node and broadcast it for all nodes</td>
    <td>The size of the container `id` should be 1. Also note this is a synchronized and blocking call so if we don't use it carefully, it can cause a deadlock.</td>
  </tr>
</table> 


## Logging strategies
Logging strategies can be helpful for recording the result of an optimization experiment. You can save the result on local file system or a remote server which is useful for distributed optimization.
<table>
  <tr>
    <th style="width:350px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`log::local::best(id, log_handler)`</td>
    <td>Stores the value of the best solution in container `id` in a local file. To create the `log_handler` see @link rocky::zagros::local_log_handler </td>
    <td></td>
  </tr>
  <tr>
    <td>`log::comet::best(id, log_handler)`</td>
    <td>Stores the value of the best solution in container `id` on [Comet](https://www.comet.ml/) server. You need to specify your Comet access token in `log_handler`. To create `log_handler` see @link rocky::zagros::comet_log_handler </td>
    <td></td>
  </tr>
</table>

## Analyzing strategies
A set of helper strategies which can be useful for analyzing the objective function and optimizers.
<table>
  <tr>
    <th style="width:350px">Name</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>`analyze::plot::heatmap(filename)`</td>
    <td>Stores the 2D mesh of the objective function is a file wich can be used to plot the surface or the contour lines of the objective function.</td>
    <td>The size of block in a blocked runtime should be 2 for using this strategy</td>
  </tr>
</table>