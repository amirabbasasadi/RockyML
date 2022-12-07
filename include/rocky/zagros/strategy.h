#ifndef ROCKY_ZAGROS_STRATEGY
#define ROCKY_ZAGROS_STRATEGY
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<tbb/tbb.h>
#include<chrono>
#include<Fastor/Fastor.h>

#include<rocky/zagros/system.h>
#include<rocky/zagros/benchmark.h>
#include<rocky/zagros/logging.h>

namespace rocky{
namespace zagros{

/**
 * @brief Interface for evolution strategies
 * 
 */
template<typename T_e, int T_dim, int T_tribe_size>
class strategy{
public:
    // sampling a particle
    static int sample_particle(){
        static thread_local std::mt19937 gen;
        std::uniform_int_distribution<> dist(0, T_tribe_size-1);
        return dist(gen);
    }
    // gaussian noise
    static T_e gaussian_noise(){
        static thread_local std::mt19937 gen;
        std::normal_distribution<T_e> dist;
        return dist(gen);
    }
    // random dimension
    static int random_dim(){
        static thread_local std::mt19937 gen;
        std::uniform_int_distribution<> dist(0, T_dim-1);
        return dist(gen);
    }
    virtual void apply(system<T_e, T_dim>* sys, T_e* population, T_e* values) = 0;
};

/**
 * @brief Gaussian mutation
 * 
 */
template<typename T_e, int T_dim, int T_tribe_size, int T_n_dim>
class gaussian_mutation: public strategy<T_e, T_dim, T_tribe_size>{
public:
    virtual void apply(system<T_e, T_dim>* sys, T_e* population, T_e* values) override{
        // pick a random particle
        int p1 = this->sample_particle();
        // selected a few dimensions randomly and make a copy of the affected dimensions
        int* dims = new int[T_n_dim];
        T_e* dims_cpy = new T_e[T_n_dim];
        for(int i=0; i<T_n_dim; i++){
            int d = this->random_dim();
            dims[i] = d;
            dims_cpy[i] = population[p1*T_dim + d];
            // tweak the dimension
            population[p1*T_dim + d] += this->gaussian_noise();
        }
        // evaluate mutated particle
        T_e new_value = sys->objective(population + p1*T_dim);
        // keep the new values if the particle has been improved
        if(new_value > values[p1]){
            values[p1] = new_value;
        }else{
            // replace the old values otherwise
            for(int i=0; i<T_n_dim; i++)
                 population[p1*T_dim + dims[i]] = dims_cpy[i];
        }
        delete[] dims;
        delete[] dims_cpy;
    }
};

};
};


#endif