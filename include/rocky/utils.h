/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_UTILS
#define ROCKY_UTILS
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<chrono>
#include<thread>
#include<functional>

#ifdef ROCKY_USE_MPI
#include<mpi.h>
#endif

#include<tbb/tbb.h>
#include<Eigen/Core>

namespace rocky{
namespace utils{

class thread_safe_prng{
public:
    std::mt19937 prng_;
    thread_safe_prng(){
        prng_.seed(static_cast<unsigned>(time(0)) + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    }
    std::mt19937& prng(){
       return prng_; 
    }
};

class random{
public:
    static std::mt19937& prng(){
        static tbb::enumerable_thread_specific<thread_safe_prng> thread_prng;
        return thread_prng.local().prng();
    }
    // generate a uniform random variable
    template<typename T_e>
    static T_e uniform(float a=0.0, float b=1.0){
        static std::uniform_real_distribution<T_e> dist(a, b);
        return dist(prng());
    }
};

};
};
#endif