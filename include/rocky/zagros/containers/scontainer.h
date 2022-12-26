#ifndef ROCKY_ZAGROS_SCONTAINER_GUARD
#define ROCKY_ZAGROS_SCONTAINER_GUARD

#include<iostream>
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<vector>
#include<set>

#include<tbb/tbb.h>
#include<Eigen/Core>

#include<rocky/zagros/benchmark.h>
#include<rocky/utils.h>

namespace rocky{
namespace zagros{
/**
 * @brief a data container representing a scontainer
 * 
 */
template<typename T_e, int T_dim>
class basic_scontainer{
protected:
    int n_particles_;
    int group_size_;
public:
    basic_scontainer(int n_particles, int group_size){
        n_particles_ = n_particles;
        group_size_ = group_size;
    }
    int n_particles() const{
        return n_particles_;
    }
    int group_size() const{
        return group_size_;
    }
    int n_groups() const{
        return n_particles() / group_size();
    }
    // holding particles
    std::vector<std::vector<T_e>> particles;
    // holding the particles value
    std::vector<T_e> values;
    // allocate the requred memory
    void allocate(){
        particles.resize(n_particles());
        for(int p=0; p<n_particles(); ++p)
            particles[p].resize(T_dim);
        values.resize(n_particles());
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
     * @brief get the group of a particle
     * 
     * @param p particle index
     * @return * int 
     */
    int particle_group(int p){
        return p / group_size();
    }
    /**
     * @brief get the address to the starting point of a group
     * 
     * @param g group's index
     * @return * T_e* 
     */
    T_e* group(int g){
        return particles[g * group_size()].data();
    }
    /**
     * @brief starting and endind point of a group
     * 
     * @param g group index
     * @return * std::pair<int, int> 
     */
    std::pair<int, int> group_range(int g) const{
        int group_s = g * group_size();
        int group_e = group_s + group_size();
        if (g == n_groups() - 1)
            group_e = n_particles();
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
    /**
     * @brief sample n distinct particles from a group
     * efficient when n is small
     * @param n sample size
     * @param group particles will be sampled from this group
     */
    std::set<int> sample_n_particles(int n, int group){
        auto group_rng = group_range(group);
        std::uniform_int_distribution<> dist(group_rng.first, group_rng.second-1);
        std::set<int> indices;
        do{
            indices.insert(dist(rocky::utils::random::prng));
        }while(indices.size() < n);
        return indices;
    }
    /**
     * @brief sample a pair of distinct particles
     * 
     * @param group 
     * @return * std::pair<int, int> 
     */
    std::pair<int, int> sample_pair(int group){
        auto indices = sample_n_particles(2, group);
        auto el = indices.begin();
        auto result = std::make_pair(*el, *(std::next(el)));
        return result;
    }
    /**
     * @brief sample a single particle from a group
     * 
     * @param group 
     * @return * int index of the particle
     */
    int sample_particle(int group){
        auto group_rng = group_range(group);
        std::uniform_int_distribution<> dist(group_rng.first, group_rng.second-1);
        return dist(rocky::utils::random::prng);
    }
    /**
     * @brief choose a dimension randomly
     * 
     * @return * int 
     */
    int sample_dim(){
        std::uniform_int_distribution<> dist(0, T_dim-1);
        return dist(rocky::utils::random::prng);
    }  
    /**
     * @brief amount of allocated memory in bytes
     * 
     */
    size_t space() const{
        return sizeof(T_e) * (n_particles() * (T_dim + 1));
    } 
};

}; // end of zagros namespace
}; // end of rocky namespace
#endif