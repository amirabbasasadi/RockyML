#ifndef ROCKY_BENCHMARK_GUARD
#define ROCKY_BENCHMARK_GUARD
#define _USE_MATH_DEFINES

#include<rocky/zagros/system.h>
#include<string>
#include<sstream>
#include<cmath>
#include<fstream>

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
    virtual std::string to_string(){
        std::stringstream name;
        name << "Sphere(dim=" << T_dim << ")";
        return name.str();
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
    virtual T_e lower_bound(){ return -5.12; }
    virtual T_e upper_bound(){ return 5.12; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "Rastrigin(dim=" << T_dim << ")";
        return name.str();
    }
};

}; // namespace benchmark
        
}; // namespace zagros
    
}; // namespace rocky

#endif