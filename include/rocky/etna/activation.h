#ifndef ROCKY_ETNA_ACT
#define ROCKY_ETNA_ACT

#include <Eigen/Core>
#include <type_traits>
#include <algorithm>
#include <cmath>

namespace rocky{
namespace etna{
namespace act{

/** 
* @brief base class for the activation functions 
*/
template<typename T_e>
class activation{};

/**
 * @brief base class for element wise acitvation functions
 * 
 */
template<typename T_e>
class elm_activation: public activation<T_e>{
public:
    /**
     * @brief evaluate activation for a single element
     * 
     * @param elm 
     * @return T_e 
     */
    virtual T_e eval(T_e elm){
        return elm;
    }
    /**
     * @brief overloaded evaluation
     * 
     * @param elm 
     * @return * T_e 
     */
    virtual T_e operator()(T_e elm){
        return eval(elm);
    }
    /**
     * @brief vector-wise activation
     * 
     * @param mem_in_ptr 
     * @param mem_out_ptr 
     * @param size 
     * @return * void 
     */
    virtual void operator()(T_e* mem_in_ptr, T_e* mem_out_ptr, int size){
        for(int i=0; i<size; i++)
            mem_out_ptr[i] = eval(mem_in_ptr[i]);
    }
    /**
     * @brief inplace vector-wise activation
     * 
     * @param mem_inout_ptr 
     * @param size 
     * @return * void 
     *
     */
    virtual void operator()(T_e* mem_inout_ptr, int size){
        (*this)(mem_inout_ptr, mem_inout_ptr, size);
    }
};


/**
 * @brief step function
 * 
 */
template<typename T_e>
class step: public elm_activation<T_e>{
public:
    virtual T_e eval(T_e elm){
        if (elm >= 0)
            return 1.0;
        return 0.0;
    }
};

};

};

};

#endif