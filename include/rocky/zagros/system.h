#ifndef ROCKY_ZAGROS_SYSTEM_GUARD
#define ROCKY_ZAGROS_SYSTEM_GUARD
namespace rocky{
namespace zagros{

template<typename T_e>
class system{
public:
    virtual T_e objective(T_e* params) = 0;
};

}; // end of zagros namespace
}; // end of rocky namespace
#endif