#ifndef ROCKY_ZAGROS_SYSTEM_GUARD
#define ROCKY_ZAGROS_SYSTEM_GUARD
#include<iostream>
#include<string>
namespace rocky{
namespace zagros{

template<typename T_e, size_t T_dim>
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

}; // end of zagros namespace
}; // end of rocky namespace
#endif