# RockyML - A High-Performance Toolkit for Non-Differentiable Learning Machines
![C++ 17](https://img.shields.io/badge/C%2B%2B-17-blueviolet?style=flat)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/amirabbasasadi/RockyML/CTest?label=build%20%26%20test)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/amirabbasasadi/RockyML)
![GitHub](https://img.shields.io/github/license/amirabbasasadi/RockyML)  

## Components  
### Rocky (Core)
Building blocks for designing non-differentiable deep architectures:
- Standard Layers
- Discrete and Combinatorial Layers
- Stochastic Layers
- Dynamic Layers

### Zagros
A high-performance and scalable black-box optimizer
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