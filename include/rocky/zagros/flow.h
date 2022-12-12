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
    Eigen::Matrix<T_e, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> chain_;
    std::vector<basic_strategy<T_e, T_dim>*> strategies_;
    int n_states_;
    int state_;
public:
    void allocate(int n_states){
        this->n_states_ = n_states;
        chain_.resize(n_states, n_states);
    }
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
        auto probs = chain_.row(state_).data();
        std::discrete_distribution<> dist(probs, probs + n_states_);
        state_ = dist(rocky::utils::random::prng);
    }
    // add a strategy to flow
    void add_strategy(basic_strategy<T_e, T_dim>* st){
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