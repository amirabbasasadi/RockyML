#ifndef ROCKY_ZAGROS_SWARM_GUARD
#define ROCKY_ZAGROS_SWARM_GUARD

#include<mpi.h>

#include<iostream>
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<vector>

#include<tbb/tbb.h>
#include<Eigen/Core>

#include<rocky/zagros/benchmark.h>

namespace rocky{
namespace zagros{
/**
 * @brief a data container representing a swarm
 * 
 */
template<typename T_e, int T_dim, int T_n_particles, int T_group_size>
struct basic_swarm{
public:
    // holding particles
    std::vector<std::vector<T_e>> particles;
    // holding the particles value
    std::vector<T_e> values;
    // allocate the requred memory
    void allocate(){
        particles.resize(T_n_particles);
        for(int p=0; p<T_n_particles; ++p)
            particles[p].resize(T_dim);
        values.resize(T_n_particles);
        // initialize particles value
        std::fill(values.begin(), values.end(), std::numeric_limits<T_e>::max());
    }
    /**
     * @brief get the starting address of a specific particle
     * 
     * @param p index of the particle
     * @return * T_e* 
     */
    T_e* particle(int p){
        return particles[p].data();
    }
    /**
     * @brief get the address to the starting point of a group
     * 
     * @param g group's index
     * @return * T_e* 
     */
    T_e* group(int g){
        return particles[g * T_group_size].data();
    }
    /**
     * @brief starting and endind point of a group
     * 
     * @param t group index
     * @return * std::pair<int, int> 
     */
    std::pair<int, int> group_range(int t) const{
        int group_s = t * T_group_size;
        int group_e = group_s + T_group_size;

        return std::make_pair(group_s, group_e);            
    }
    /**
     * @brief pointer to the value of a particle
     * 
     * @return * T_e* 
     */
    T_e* value(int p){
        return &values[p];
    }
};

}; // end of zagros namespace
}; // end of rocky namespace
#endif