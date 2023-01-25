cmake_minimum_required(VERSION 3.2)
project(RockyML LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -march=native")

set(ROCKY_BUILD_MPI_TESTS TRUE)


include_directories(include/)

find_package(TBB CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)

set(ROCKY_SRC src/rocky/utils.cc src/rocky/zagros/dena.cc)
add_library(rockyml STATIC ${ROCKY_SRC})
target_link_libraries(rockyml PRIVATE Catch2::Catch2 TBB::tbb TBB::tbbmalloc Eigen3::Eigen cpr::cpr spdlog::spdlog)


add_executable(tests tests/catch_main.cc tests/scontainer.cc tests/flow.cc tests/strategy.cc)
target_link_libraries(tests PRIVATE Catch2::Catch2 TBB::tbb TBB::tbbmalloc Eigen3::Eigen cpr::cpr spdlog::spdlog rockyml)

if(ROCKY_BUILD_MPI_TESTS)
# tests that require MPI
add_executable(mpi_tests tests/mpi/basic_strategy.cc)
target_link_libraries(mpi_tests TBB::tbb TBB::tbbmalloc Eigen3::Eigen cpr::cpr spdlog::spdlog rockyml)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)