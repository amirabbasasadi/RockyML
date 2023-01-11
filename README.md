<p><img align="center" src="/logo/rockyml-white-1200.png"></p>
<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blueviolet?style=flat">
  <img src="http://img.shields.io/github/actions/workflow/status/amirabbasasadi/RockyML/ctest.yml?branch=main">
  <img src="https://img.shields.io/github/commit-activity/m/amirabbasasadi/RockyML">
  <img src="https://img.shields.io/github/license/amirabbasasadi/RockyML">
</p>  

**Work in Progress!! Wait until a stable release!**

- **Zagros** : A parallel and distributed optimization engine
  - **Dena** : A language for designing search strategies
- **Etna** : Building blocks for designing fast non-differentiable neural networks


## Zagros
<p align="center"><img src="/logo/zagros-transparent-600.png"></p>  

Design Goals:
- CPU-friendly: targeting multi-core systems and clusters
- Hybrid parallelism: multi-threading in each node and message passing across nodes (MPI)
- Handling large number of variables using block coordinate descent
- Design arbitrary complex optimizers by combining modular search/communication strategies


## Etna
Building blocks for designing non-differentiable neural networks

- Fast, low overhead, and thread-safe 
- Various components:
  - Standard deep learning layers
  - Discrete and integer layers
  - Combinatorial layers
  - Stochastic layers
  - Dynamic layers
  
## Guide
### Dependencies
For usage:
- A C++ compiler supporting C++17, GCC is tested
- Eigen
- A BLAS/LAPACK implementation, OpenBLAS is tested
- Intel Threading Building Blocks (TBB)
- An MPI implementation, OpenMPI is recommended

Development only:  
- Catch2
- Nanobench
