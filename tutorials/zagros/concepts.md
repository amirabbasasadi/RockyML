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

### Swarm
Most meta-huristic search algorithms such as Genetic algorithms, PSO, differential evoultion and ... are based on iteratively tweaking a set of solutions to obtain a satisfying result. In Zagros it's called *swarm* while in context of Genetic Algorithms is known as *population*. Whatever we call it, It represents the set of potential solutions. Since Zagros is a large-scale optimizer, It distributes the population across multiple nodes. We call each memeber of this population a *particle*.

#### Groups
To increase exploration, Some meta-huristic approaches divide the swarm into specific subsets. For instance *TribesPSO* splits the swarm in groups called *Tribe*. Note in Zagros, all particles of a *group* must reside on the same node. Although this division is often hierd to avoid being trapped in local minimums and increasing exploration, Zagros orginizes particles in groups to achieve thread-safety.

### Strategies
A *strategy* is any procedure that changes the current swarm. In Zagros many optimization sub-routines are defined as strategies. For example:
- Initialization strategies : responsible to generate an initial swarm
- Search strategies: represent the logic of an specific optimization algorithm. For example a mutation strategy may add a random value to a particle.  
- Communication strategies: particles on differenti nodes can communicate to share their solutions or other information of search space though patterns specified by communication strategies. Such patterns can be local or global across the whole cluster.

### Flows
In Zagros, A *Flow* specifies a control policy to apply a set of strategies on a swarm. Under the hood a flow is a markov chain that describes the transition probabilities among a set of strategies. 