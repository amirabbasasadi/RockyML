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
template<typename T_e>
class sphere: public rocky::zagros::system<T_e>{
protected:
    int dim_;
public:
    sphere(int dim){
        dim_ = dim;
    }
    virtual T_e objective(T_e* x){
        T_e S = 0.0;
        for(size_t i=0; i<dim_; i++)
            S += x[i] * x[i];
        return sqrt(S);
    }
    virtual T_e lower_bound(){ return -10.0; }
    virtual T_e upper_bound(){ return 10.0; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "Sphere(dim=" << dim_ << ")";
        return name.str();
    }
};

/**
 * @brief Rastrigin function
 * 
 */
template<typename T_e>
class rastrigin: public rocky::zagros::system<T_e>{
protected:
    int dim_;
public:
    rastrigin(int dim=2){
        dim_ = dim;
    }
    virtual T_e objective(T_e* x){
        T_e S = 10.0 * dim_;
        for(size_t i=0; i<dim_; i++)
            S += x[i] * x[i] - 10.0*cos(2*M_PI * x[i]);
        return S;
    }
    virtual T_e lower_bound(){ return -5.12; }
    virtual T_e upper_bound(){ return 5.12; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "Rastrigin(dim=" << dim_ << ")";
        return name.str();
    }
};

}; // namespace benchmark
        
}; // namespace zagros
    
}; // namespace rocky

#endif