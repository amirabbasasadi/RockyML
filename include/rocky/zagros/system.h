/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_ZAGROS_SYSTEM_GUARD
#define ROCKY_ZAGROS_SYSTEM_GUARD
#include<iostream>
#include<string>
#include<memory>

#include<tbb/tbb.h>

#include "spdlog/spdlog.h"


namespace rocky{
namespace zagros{

template<typename T_e>
class system{
public:
    virtual T_e objective(T_e* params) = 0;
    /**
     * @brief lower bound specification
     * should be used when lower bound is same for all parameters
     * @return ** T_e
     */
    virtual T_e lower_bound(){ return -1.0;}
    /**
     * @brief lower bound specification for each parameter
     *  should be used if parameters have different lower bounds
     * @param p_index 
     * @return ** T_e 
     */
    virtual T_e lower_bound(int p_index){ return lower_bound();}
    /**
     * @brief upper bound specification
     * should be used when upper bound is same for all parameters
     * @return ** T_e
     */
    virtual T_e upper_bound(){ return 1.0;}
    /**
     * @brief upper bound specification for each parameter
     *  should be used if parameters have different upper bounds
     * @param p_index 
     * @return ** T_e 
     */
    virtual T_e upper_bound(int p_index){ return upper_bound();}
    virtual std::string to_string(){
        return "optimization problem";
    }
    /**
     * @brief optimize objective calculation for a block
     * for each new bcd mask this function will be calles
     * so we can optimize objective computation based on the mask
     * 
     * @param block_mask coordinae descent mask
     * @param block_dim block dim for block optimization
     * @return * void 
     */
    virtual void optimize_for_block(int* block_mask, int block_dim){}
};

/**
 * @brief a virtual system to implement blocked coordinate descent
 * 
 */
template<typename T_e>
class blocked_system: public system<T_e>{
public:
    int block_dim_;
    int original_dim_;
    // thread-specific solution states provided by the runtime
    tbb::enumerable_thread_specific<std::vector<T_e>>* solution_state_;
    // main system
    system<T_e>* main_system_;
    // block mask
    int* bcd_mask_;
    
    int original_dim() const{
        return original_dim_;
    }

    int block_dim() const{
        return block_dim_;
    }

    blocked_system(system<T_e>* main_system, int original_dim, int block_dim, int* mask){
        this->main_system_ = main_system;
        this->original_dim_ = original_dim;
        this->block_dim_ = block_dim;
        this->bcd_mask_ = mask;
    }
    // change the solution state
    void set_solution_state(tbb::enumerable_thread_specific<std::vector<T_e>>* solution_state){
        this->solution_state_ = solution_state;
    }
    virtual T_e objective(T_e* partial){
        // get a thread specific solution
        T_e* full_solution = this->solution_state_->local().data();
        // copy the partial solution to the full solution
        for(int i=0; i<block_dim_; i++)
            full_solution[bcd_mask_[i]] = partial[i];
        // evaluate the full solution
        return main_system_->objective(full_solution);
    }
    /**
     * @brief lower bound specification
     * should be used when lower bound is same for all parameters
     * @return ** T_e
     */
    virtual T_e lower_bound(){ return this->main_system_->lower_bound();}
    /**
     * @brief lower bound specification for each parameter
     *  should be used if parameters have different lower bounds
     * @param p_index 
     * @return ** T_e 
     */
    virtual T_e lower_bound(int p_index){ return this->main_system_->lower_bound(bcd_mask_[p_index]);}
    /**
     * @brief upper bound specification
     * should be used when upper bound is same for all parameters
     * @return ** T_e
     */
    virtual T_e upper_bound(){ return this->main_system_->upper_bound();}
    /**
     * @brief upper bound specification for each parameter
     *  should be used if parameters have different upper bounds
     * @param p_index 
     * @return ** T_e 
     */
    virtual T_e upper_bound(int p_index){ return this->main_system_->upper_bound(bcd_mask_[p_index]);}
    /**
     * @brief optimize the wrapped system for block runtime
     * 
     */
    virtual void optimization_for_block(){
        this->main_system_->optimize_for_block(this->bcd_mask_, this->block_dim_);
    }
};

}; // end of zagros namespace
}; // end of rocky namespace
#endif