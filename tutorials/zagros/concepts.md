# Getting Started with Zagros

## Design Goals
A large-scale black-box optimizer targeting multi-core clusters:    
- Hybrid parallelism : multi-threading for a single node, message passing across nodes  
- Unifying various search algorithms   
- Combining available or user-defined search strategies to construct arbitrary complex optimizers  
- Optimizing large number of parameters through block coordinate descent  

## Concept and Components

### System
In Zagros a *system* defines an optimization problem. A system esentially provides a thread-safe way to evaluate the objective function. Moreover a system can optionally provide the followings:  
- Search region
- Problem constraints 
- Hints for generating feasible soltuions or evolving the existing ones 

### Population
Most meta-huristic search algorithms such as Genetic algorithms, PSO, differential evoultion and ... are based on tweaking a set of solutions to obtain a satisfying result. Whatever we call it, It represents the set of potential solutions. Since Zagros is a large-scale optimizer, It distributes the population across multiple nodes. We call each memeber of this population a *particle*.

#### Groups
To increase exploration, Some meta-huristic approaches divide the population into specific subsets. For instance *TribesPSO* splits the swarm in groups called *Tribe*. Note in Zagros, all particles of a *group* must reside on the same node. Although this division is often hierd to avoid being trapped in local minimums and increasing exploration, Zagros orginizes particles in groups to achieve thread-safety.

### Search Strategies
After initializing the population, every algorithm suggusts its own way to evolve the particles. Particle Swarm updates the particles using a set of velocity vectors, Genetic Algorithms use mutation and cross-over, Differential Evolution uses the differnece of current particles and so on. In Zagros, Any policy to change the existing particles is called a *Search Strategy*. 