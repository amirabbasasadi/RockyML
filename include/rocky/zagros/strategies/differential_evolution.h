/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_ZAGROS_DE_STRATEGY
#define ROCKY_ZAGROS_DE_STRATEGY

#include <rocky/zagros/strategies/strategy.h>

namespace rocky{
namespace zagros{

template<typename T_e, int T_dim>
class basic_differential_evolution: public search_strategy<T_e, T_dim>{
protected:
    system<T_e>* problem_;
    // target container
    basic_scontainer<T_e, T_dim>* container_;
    // candidates container
    basic_scontainer<T_e, T_dim>* candidates_;
    // DE hyper-parameters
    T_e CR_;
    T_e DW_;
    // number of crossovers
    int n_crossovers_;

public:
    basic_differential_evolution(system<T_e>* problem, basic_scontainer<T_e, T_dim>* container, basic_scontainer<T_e, T_dim>* candidates, T_e CR=0.9, T_e DW=0.9){
        this->problem_ = problem;
        this->container_ = container;
        this->candidates_ = candidates;
        this->CR_ = CR;
        this->DW_ = DW;
        this->n_crossovers_ = candidates->n_particles();
    }
    /**
     * @brief generate a uniform random number
     * 
     * @return * T_e 
     */
    T_e rand_uniform(){
        static std::uniform_real_distribution<T_e> dist(0.0, 1.0);
        return dist(rocky::utils::random::prng());
    }    
    // apply differential evolution within groups in parallel
    virtual void apply(){
        tbb::parallel_for(0, n_crossovers_, [this](int p){
            int parents[4];
            this->container_->sample_n_particles(parents, 4);
            auto& [x, a, b, c] = parents;
            // making a copy of x
            std::copy(this->container_->particle(x),
                      this->container_->particle(x) + T_dim,
                      this->candidates_->particle(p));
            for(int d=0; d<T_dim; d++){
                // perform crossover based on the crossover probability CR
                if(this->rand_uniform() > this->CR_)
                    continue;
                // apply crossover
                this->candidates_->particles[p][d] = this->container_->particles[a][d] +
                                                     this->DW_ * (this->container_->particles[b][d] -
                                                                  this->container_->particles[c][d]);            
            }
        });
        // replace improved solutions
        candidates_->evaluate_and_update(problem_);
        container_->replace_with(candidates_);
    }
};


};
};

#endif