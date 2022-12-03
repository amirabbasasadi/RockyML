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
 * @brief PSO Log
 * An object for logging the result of optimization
 * The output will be saved as a .csv file
 */
template<typename T_e>
class log_pso_tribes{
    std::ofstream output;
    int n_tribes;
    int n_particles;

public:
    log_pso_tribes(std::string path, int n_particles, int n_tribes){
        output.open(path, std::ofstream::out);
        output << "time,tribe,value" << "\n";
        this->n_particles = n_particles;
        this->n_tribes = n_tribes;
    }
    void step(size_t time, const T_e* tribes_best_min_){
        for(int t=0; t<n_tribes; t++){
            output << time << "," << t << "," << tribes_best_min_[t] << "\n";
        }
    }
    void save(){
        output.close();
    }
    ~log_pso_tribes(){
        if (output.is_open()) output.close();
    }
};


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