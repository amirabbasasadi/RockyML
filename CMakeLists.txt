cmake_minimum_required(VERSION 3.2)
project(RockyML LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 
)
FetchContent_MakeAvailable(Catch2)

include_directories(include/)
add_executable(tests test/linear.cc)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain openblas)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)