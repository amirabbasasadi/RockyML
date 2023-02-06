cmake_minimum_required(VERSION 3.2)
project(Example LANGUAGES CXX)

# make sure your compiler supports c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -march=native")

include_directories(../../include/)

# find requirements
find_package(TBB CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# linking the requirements
add_executable(app main.cc)
target_link_libraries(app PRIVATE TBB::tbb TBB::tbbmalloc Eigen3::Eigen cpr::cpr spdlog::spdlog nlohmann_json::nlohmann_json)
