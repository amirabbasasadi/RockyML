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

namespace rocky{
namespace utils{

class random{
public:
    thread_local inline static std::mt19937 prng;
    static void init(unsigned int seed){ random::prng.seed(seed);}
    // generate a uniform random variable
    template<typename T_e>
    static T_e uniform(float a=0.0, float b=1.0){
        static std::uniform_real_distribution<T_e> dist(a, b);
        return dist(prng);
    }
};

};
};
#endif