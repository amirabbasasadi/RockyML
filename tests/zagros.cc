#include <catch2/catch_all.hpp>
#include <random>
#include <functional>
#include <algorithm>
#include <cmath>
#include <rocky/etna/blocks.h>
#include <rocky/zagros/system.h>

template<typename T_e, size_t T_dims>
class sphere: rocky::zagros::system<T_e,T_dims>{
public:
    virtual T_e objective(T_e* params){
        T_e S = 0.0;
        for(size_t i=0; i<T_dims; i++)
            S += params[i] * params[i];
        return sqrt(S);
    }
    virtual T_e lower_bound(int p){
        if(p < 10){
            return -1.0;
        }
        return -2.0;
    }
    virtual T_e upper_bound(int p){
        if(p < 10){
            return 1.0;
        }
        return 2.0;
    }
};

TEST_CASE("Creating systems", "[system][zagros][double]"){
    using namespace rocky;
    
    const size_t dim = 64;
    auto system = sphere<double, dim>();
    double* params = new double[dim];
    std::fill(params, params+dim, 0.0);
    REQUIRE(system.objective(params) == 0.0);
    REQUIRE(system.lower_bound(1) == -1.0);
    REQUIRE(system.lower_bound(50) == -2.0);
    REQUIRE(system.upper_bound(1) == 1.0);
    REQUIRE(system.upper_bound(50) == 2.0);
    delete[] params;
}
