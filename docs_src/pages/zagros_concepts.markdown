# What is Zagros?{#zagros_concepts}

## Design Goals
- Providing a language called Dena for designing arbitrary complex optimizers by combining 
  - Modular and parallel search strategies: Genetic, PSO, EDA, ...
  - Communication strategies for distributed optimization on top of MPI
  - Analyzer strategies for analyzing objective functions
  - Blocking strategies for block optimization
  - Logging strategies for tracking optimization experiments on local system or a remote server
- Hybrid parallelism: multi-threading in each node and message passing across nodes (MPI)
- ‌Block optimization for using memory-intensive optimizers for large number of variables