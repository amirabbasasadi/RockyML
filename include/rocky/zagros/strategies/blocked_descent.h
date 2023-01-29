/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_ZAGROS_BCD_STRATEGY
#define ROCKY_ZAGROS_BCD_STRATEGY
#include <rocky/zagros/strategies/strategy.h>

#include<set>

namespace rocky{
namespace zagros{

/**
 * @brief Interface for blocked descent strategies
 * 
 */
template<typename T_e, int T_dim>
class bcd_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief Interface for bcd mask generation strategies
 * 
 */
template<typename T_e, int T_dim>
class bcd_mask_gen_strategy: public bcd_strategy<T_e, T_dim>{};

/**
 * @brief Interface for bcd synchronization strategies
 * 
 */
template<typename T_e, int T_dim>
class bcd_sync_strategy: public bcd_strategy<T_e, T_dim>{};

/**
 * @brief Uniform mask generator
 * 
 */
template<typename T_e, int T_dim>
class bcd_mask_uniform_random: public bcd_mask_gen_strategy<T_e, T_dim>{
protected:
    blocked_system<T_e>* problem_;
    std::vector<int>* bcd_mask_;

int rand_dim(){
        static std::uniform_int_distribution<int> dist(0, problem_->original_dim()-1);
        return dist(rocky::utils::random::prng());
}
public:
    bcd_mask_uniform_random(blocked_system<T_e>* problem, std::vector<int>* bcd_mask){
        this->problem_ = problem;
        this->bcd_mask_ = bcd_mask;
    }
    virtual void apply(){
       std::set<int> dims;
       while(dims.size() < this->problem_->block_dim())
            dims.insert(this->rand_dim());
       int i = 0;
       for(auto dim: dims)
           bcd_mask_->at(i++) = dim;
    };
};


};
};
#endif