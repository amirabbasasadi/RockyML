#include <catch2/catch_all.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <cmath>
#include <rocky/blocks.h>
#include <rocky/zagros/system.h>

template<typename T_e, size_t T_dims>
class sphere: rocky::zagros::system<T_e>{
public:
    virtual T_e objective(T_e* params){
        T_e S = 0.0;
        for(size_t i=0; i<T_dims; i++)
            S += params[i] * params[i];
        return sqrt(S);
    }
};

TEST_CASE("Creating systems", "[system][zagros][double]"){
    using namespace rocky;
    
    const size_t dim = 64;
    auto system = sphere<double, dim>();
    double* params = new double[dim];
    std::fill(params, params+dim, 0.0);
    REQUIRE(system.objective(params) == 0.0);

    delete[] params;
}
