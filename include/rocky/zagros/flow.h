#ifndef ROCKY_ZAGROS_FLOW_GUARD
#define ROCKY_ZAGROS_FLOW_GUARD

#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<chrono>

#include<tbb/tbb.h>
#include<Eigen/Core>

#include<rocky/zagros/system.h>
#include<rocky/zagros/strategies/strategy.h>
#include <rocky/utils.h>


namespace rocky{
namespace zagros{

/**
 * @brief base class for optimization Flows
 * 
 */
template<typename T_e, int T_dim>
class basic_flow{
protected:
    basic_strategy<T_e, T_dim>* state_;
public:
    // get the current state
    basic_strategy<T_e, T_dim>* state(){
        return state_;
    }
    // set the running state
    void set_state(basic_strategy<T_e, T_dim>* state){
        this->state_ = state;
    }
    // take one step and move to another state
    virtual void step(){
        if(state())
            this->state()->apply();
    }
};

};
};
#endif