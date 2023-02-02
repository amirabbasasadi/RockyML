# Using RockyML with CMake{#build}

RockyML is a **header-only** library. So you just need to include it in your project. But first make sure you have installed the required dependencies:
## Dependencies
- A C++ compiler supporting C++17, GCC is tested
- CMake
- [Eigen](https://eigen.tuxfamily.org), latest stable version
- A BLAS/LAPACK implementation, OpenBLAS is tested
- Intel Threading Building Blocks ([oneTBB](https://github.com/oneapi-src/oneTBB))
- An MPI implementation, OpenMPI is recommended
- [nlohmann/json](https://github.com/nlohmann/json)  
- [Cpr](https://github.com/libcpr/cpr)
- [Spdlog](https://github.com/gabime/spdlog)

First install Open MPI and OpenBLAS:
```
apt install libopenblas-dev openmpi-bin openmpi-common libopenmpi-dev
```
It is highly recommended to install the other dependencies using a package manager like [vcpkg](https://vcpkg.io/):
```
vcpkg install tbb eigen3 cpr spdlog nlohmann-json catch2
```
## A minimal CMake example
Here is a minimal CMakeLists.txt example for using RockyML:  
```cmake
cmake_minimum_required(VERSION 3.2)
project(Example LANGUAGES CXX)

# make sure your compiler supports c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -march=native")

include_directories(<path_to_rockyml_include_folder>)

# find requirements
find_package(TBB CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

# linking the requirements
add_executable(app main.cc)
target_link_libraries(app PRIVATE TBB::tbb TBB::tbbmalloc Eigen3::Eigen cpr::cpr spdlog::spdlog lohmann_json::nlohmann_json)
```

**Note** : if you've used vcpkg to install requirements, make sure that you set the `CMAKE_TOOLCHAIN_FILE`.

```shell
cmake .. -DCMAKE_CXX_COMPILER=mpic++ -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg.cmake>
```
Finally you can build your project:
```
make
```
