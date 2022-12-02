#ifndef ROCKY_BENCHMARK_GUARD
#define ROCKY_BENCHMARK_GUARD
#define _USE_MATH_DEFINES

#include<rocky/zagros/system.h>
#include<cmath>

namespace rocky{
namespace zagros
{
namespace benchmark
{

/**
 * @brief Sphere function
 * 
 */
template<typename T_e, size_t T_dim>
class sphere: public rocky::zagros::system<T_e, T_dim>{
public:
    virtual T_e objective(T_e* x){
        T_e S = 0.0;
        for(size_t i=0; i<T_dim; i++)
            S += x[i] * x[i];
        return sqrt(S);
    }
};

/**
 * @brief Rastrigin function
 * 
 */
template<typename T_e, size_t T_dim>
class rastrigin: public rocky::zagros::system<T_e, T_dim>{
public:
    virtual T_e objective(T_e* x){
        T_e S = 10.0 * T_dim;
        for(size_t i=0; i<T_dim; i++)
            S += x[i] * x[i] - 10.0*cos(2*M_PI * x[i]);
        return S;
    }
};

}; // namespace benchmark
        
}; // namespace zagros
    
}; // namespace rocky

#endif