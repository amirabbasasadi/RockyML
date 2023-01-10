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
    blocked_system(system<T_e>* main_system, int original_dim, int block_dim){
        this->main_system_ = main_system;
        this->original_dim_ = original_dim;
        this->block_dim_ = block_dim;
    }
    // change the solution state
    void set_solution_state(tbb::enumerable_thread_specific<std::vector<T_e>>* solution_state){
        this->solution_state_ = solution_state;
    }
    // map a partial parameter index to the original index in the full solution vector
    virtual int partial_map(int p_index){
        return p_index;
    }
    virtual T_e objective(T_e* partial){
        // get a thread specific solution
        T_e* full_solution = this->solution_state_->local().data();
        // copy the partial solution to the full solution
        for(int i=0; i<block_dim_; i++)
            full_solution[partial_map(i)] = partial[i];
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
    virtual T_e lower_bound(int p_index){ return this->main_system_->lower_bound(partial_map(p_index));}
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
    virtual T_e upper_bound(int p_index){ return this->main_system_->upper_bound(partial_map(p_index));}
};

}; // end of zagros namespace
}; // end of rocky namespace
#endif