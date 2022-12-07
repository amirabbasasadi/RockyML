cmake_minimum_required(VERSION 3.2)
project(RockyML LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")

set(ROCKY_BUILD_MPI_TESTS FALSE)

Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG v4.1.0
    GIT_SHALLOW TRUE)


FetchContent_MakeAvailable(nanobench)

include_directories(include/)

find_package(TBB CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

add_executable(tests tests/zagros.cc tests/linear.cc tests/strategy.cc)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain TBB::tbb TBB::tbbmalloc Eigen3::Eigen)

if(ROCKY_BUILD_MPI_TESTS)
# tests that require MPI
add_executable(mpi_tests tests/mpi/basic_mpi.cc)
target_link_libraries(mpi_tests nanobench TBB::tbb TBB::tbbmalloc Eigen3::Eigen)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)