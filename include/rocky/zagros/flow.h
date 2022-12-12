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
#include<rocky/zagros/strategy.h>
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
    Eigen::Matrix<T_e, Eigen::Dynamic, Eigen::Dynamic> chain_;
    std::vector<strategy<T_e, T_dim>*> strategies_;
    int state_;
public:
    void allocate(int max_strategies)[
        chain_.resize(max_strategies, max_strategies);
    ]
    int state() const{
        return state_;
    }
    void set_state(int state_id){
        state_ = state_id;
    }
    void set_transition(int src, int dest, T_e prob){
        chain_(src, dest) = prob;
    }
    // take one step and move to another state
    void step(){
        // [todo]
    }
    // add a strategy to flow
    void add_strategy(strategy<T_e, T_dim>* st){
        strategies_.push_back(st);
    }
    // normalize the transition matrix
    void normalize_chain(){
        // [todo]
    }
};

};
};
#endif