/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_ZAGROS_CONTAINER_MAN
#define ROCKY_ZAGROS_CONTAINER_MAN

#include <rocky/zagros/strategies/strategy.h>

namespace rocky{
namespace zagros{

/**
 * @brief base class for strategies for container manipulation
 * 
 * @tparam T_e 
 * @tparam T_dim 
 */
template<typename T_e, int T_dim>
class container_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief replace worst solutions in a container with the best solutions of another
 * 
 */
template<typename T_e, int T_dim>
class select_from_strategy: public container_strategy<T_e, T_dim>{
protected:
    // destination container
    basic_scontainer<T_e, T_dim>* des_;
    // source container
    basic_scontainer<T_e, T_dim>* src_;
    
public:
    select_from_strategy(basic_scontainer<T_e, T_dim>* des, basic_scontainer<T_e, T_dim>* src){
        des_ = des;
        src_ = src;
    }
    virtual void apply(){
        des_->replace_with(src_);
    }
};

};
};

#endif