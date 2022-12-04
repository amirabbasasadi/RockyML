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
    virtual std::string to_string(){
        return "optimization problem";
    }
};

}; // end of zagros namespace
}; // end of rocky namespace
#endif