#ifndef ROCKY_ZAGROS_STRATEGY
#define ROCKY_ZAGROS_STRATEGY
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<chrono>

#include "spdlog/spdlog.h"

#include<tbb/tbb.h>
#include<Eigen/Core>

#include<rocky/zagros/system.h>
#include<rocky/zagros/containers/scontainer.h>
#include <rocky/utils.h>


namespace rocky{
namespace zagros{
/**
 * @brief Interface for all strategies
 * 
 */
template<typename T_e, int T_dim>
class basic_strategy{
public:
    virtual void apply() = 0;
};

/**
 * @brief Interface for evolution strategies
 * 
 */
template<typename T_e, int T_dim>
class search_strategy: basic_strategy<T_e, T_dim>{
public:
    virtual void apply() = 0;
};

};
};
#endif