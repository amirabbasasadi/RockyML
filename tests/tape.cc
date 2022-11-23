#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <rocky/tape.h>

TEST_CASE("Data Tape, Dynamic Tape", "[benchmark]") {
    using namespace rocky;
    rocky::dynamic_tape<double> t(1024);
    REQUIRE(t.size() == 1024);
}