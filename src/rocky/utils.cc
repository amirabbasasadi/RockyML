#include <rocky/utils.h>

using namespace rocky::utils;

// global thread-safe PRNG
tbb::enumerable_thread_specific<thread_safe_prng> random::thread_prng{};
