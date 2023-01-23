<p><img align="center" src="/logo/rockyml-poster-white-2500.png"></p>
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
<p align="center"><img src="/logo/zagros-transparent-300.png"></p>  

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
### Quick Start (Linux)
Before buiding RockyML make sure you have installed the required dependencies:
#### Dependencies
- A C++ compiler supporting C++17, GCC is tested
- CMake
- Eigen
- A BLAS/LAPACK implementation, OpenBLAS is tested
- Intel Threading Building Blocks (TBB)
- An MPI implementation, OpenMPI is recommended  
- cpr
- Spdlog

First install Open MPI and OpenBLAS:
```
apt install libopenblas-dev openmpi-bin openmpi-common libopenmpi-dev
```
You can install the other dependencies as you like. For example using vcpkg:
```
vcpkg install tbb eigen3 cpr spdlog
```
#### Bulding on Linux
Build the source code using CMake. To do so, create a build directory inside the source code and run cmake. you should add MPI flags for compiling, or you can simply use `mpic++` for compiling the source.  
**Note** : if you've used vcpkg to install requirements, make sure that you set the `CMAKE_TOOLCHAIN_FILE`.

```shell
cmake .. -DCMAKE_CXX_COMPILER=mpic++ -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg.cmake>
```
```
make
```
If building was successful, you should be able to find `librockyml.a`. you need to link this static library to your program.
#### Using with CMake
Here is a minimal CMakeLists.txt example for using RockyML:  
```cmake
cmake_minimum_required(VERSION 3.2)
project(Example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -march=native")

Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 
)
FetchContent_MakeAvailable(Catch2)

include_directories(<rockyml-include-directory>)

find_package(TBB CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

find_library(RockyML NAMES rockyml librockyml HINTS "<path-to-rockyml-static-library>")

add_executable(app <path-to-source-of-your-program>)
target_link_libraries(app PRIVATE TBB::tbb TBB::tbbmalloc Eigen3::Eigen cpr::cpr spdlog::spdlog RockyML)

```
