cmake_minimum_required(VERSION 3.2)
project(RockyML LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -march=native")

include_directories(../../include/)

find_package(TBB CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

add_executable(app main.cc)
target_link_libraries(app TBB::tbb TBB::tbbmalloc Eigen3::Eigen)