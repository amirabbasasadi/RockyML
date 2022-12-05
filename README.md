#  RockyML | A High-Performance Scientific Computing Framework  

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blueviolet?style=flat">
  <img src="https://img.shields.io/github/workflow/status/amirabbasasadi/RockyML/CTest?label=build%20%26%20test">
  <img src="https://img.shields.io/github/commit-activity/m/amirabbasasadi/RockyML">
  <img src="https://img.shields.io/github/license/amirabbasasadi/RockyML">
</p>

- **Etna** : Building blocks for designing fast non-differentiable neural networks
- **Zagros** : A high-performance and scalable black-box optimization engine


## Etna
Building blocks for designing non-differentiable neural networks

- Standard Deep Learning Layers
- Discrete and Combinatorial Layers
- Stochastic Layers
- Dynamic Layers

## Zagros
A high-performance and scalable black-box optimization engine

- CPU-friendly: targeting multi-core sytems and many-core clusters
- Two levels of parallelism: multi-threading, multi-processing
- Distributed optimization

## Guide
### Dependencies
For usage:
- A C++ compiler supporting C++17, GCC is tested
- Fastor
- A BLAS/LAPACK implementation, OpenBLAS is tested
- Intel Threading Building Blocks (TBB)
- An MPI implementation, OpenMPI is tested

Development only:  
- Catch2
- Nanobench
