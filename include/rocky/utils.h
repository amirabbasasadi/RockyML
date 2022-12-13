#ifndef ROCKY_STRATEGY
#define ROCKY_STRATEGY
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<chrono>

#ifdef ROCKY_USE_MPI
#include<mpi.h>
#endif

#include<tbb/tbb.h>
#include<Eigen/Core>

#include<rocky/zagros/system.h>
#include<rocky/zagros/containers/swarm.h>

namespace rocky{
namespace utils{

class random{
public:
    thread_local inline static std::mt19937 prng;
    static void init(unsigned int seed){ random::prng.seed(seed);}
};

};
};
#endif