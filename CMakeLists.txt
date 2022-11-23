cmake_minimum_required(VERSION 3.2)
project(RockyML)

set(CXX_STANDARD 17)

Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests test/)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)