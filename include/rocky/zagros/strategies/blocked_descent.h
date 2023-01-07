#ifndef ROCKY_ZAGROS_BCD_STRATEGY
#define ROCKY_ZAGROS_BCD_STRATEGY
#include <rocky/zagros/strategies/strategy.h>

namespace rocky{
namespace zagros{

/**
 * @brief Interface for blocked descent strategies
 * 
 */
template<typename T_e, int T_dim>
class bcd_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief Uniform initializer
 * 
 */
template<typename T_e, int T_dim>
class uniform_random_bcd: public bcd_strategy<T_e, T_dim>{
protected:
    blocked_system<T_e, T_dim>* problem_;

public:
    uniform_random_bcd(blocked_system<T_e, T_dim>* problem){
        this->problem_ = problem;
    }
    virtual void apply(){
       
    };
};


};
};
#endif