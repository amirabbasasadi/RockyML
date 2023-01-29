/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_ZAGROS_INIT_STRATEGY
#define ROCKY_ZAGROS_INIT_STRATEGY
#include <rocky/zagros/strategies/strategy.h>

namespace rocky{
namespace zagros{

/**
 * @brief Interface for initialization strategies
 * 
 */
template<typename T_e, int T_dim>
class init_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief Uniform initializer
 * 
 */
template<typename T_e, int T_dim>
class uniform_init_strategy: public init_strategy<T_e, T_dim>{
protected:
    system<T_e>* problem_;
    basic_scontainer<T_e, T_dim>* container_;

    T_e rand_uniform(T_e lb, T_e ub){
        std::uniform_real_distribution<T_e> dist(lb, ub);
        return dist(rocky::utils::random::prng());
    }
public:
    uniform_init_strategy(system<T_e>* problem, basic_scontainer<T_e, T_dim>* container){
        this->problem_ = problem;
        this->container_ = container; 
    }
    virtual void apply(){
        tbb::parallel_for(0, this->container_->n_particles(), [&](auto p){
            for(int d=0; d<T_dim; ++d)
               this->container_->particle(p)[d] = rand_uniform(this->problem_->lower_bound(p), this->problem_->upper_bound(p));
        });
    };
};


};
};
#endif