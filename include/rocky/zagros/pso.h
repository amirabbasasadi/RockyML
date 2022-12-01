#ifndef ROCKY_ZAGROS_PSO_GUARD
#define ROCKY_ZAGROS_PSO_GUARD
#include<iostream>
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<mpi.h>
#include<tbb/tbb.h>
#include<chrono>
#include<Fastor/Fastor.h>
#include<rocky/zagros/distributed.h>
#include<rocky/zagros/system.h>

namespace rocky{
namespace zagros{

template<typename T_e,
         int T_dim,
         int T_n_particles>
class pso_mpi: public basic_mpi_optimizer{
protected:
    zagros::system<T_e, T_dim>* problem_;
    // particles position
    T_e particles_x_[T_n_particles * T_dim];
    // particles velocity
    T_e particles_v_[T_n_particles * T_dim];
    // particles best solution
    T_e particles_best_[T_n_particles];
public:
    constexpr int n_particles() { return T_n_particles; } 
    // initialize particles position
    void initialize_position(){
        Fastor::TensorMap<T_e, T_n_particles, T_dim> X_(particles_x_);
        X_.random();
    }
    /**
     * @brief initialize best solution for each particle
     * 
     * @return ** void 
     */
    void initialize_best(){
        std::fill(particles_best_, particles_best_+n_particles(), std::numeric_limits<T_e>::max());
    }
    void add_system(zagros::system<T_e, T_dim>* sys){
        problem_ = sys;
    }
    /**
     * @brief evaluate positions of particles in parallel
     * 
     * @return ** void 
     */
    void eval_particles(){
        tbb::parallel_for(0, n_particles(), [&](int p){
            T_e obj = problem_->objective(particles_x_ + p*T_dim);
            if (obj < particles_best_[p])
                particles_best_[p] = obj;
        });
    }
    /**
     * @brief find best current solution using parallel reduction
     * 
     * @return ** std::pair<T_e, int> 
     */
    std::pair<T_e, int> best(){
        std::pair<T_e, int> best = tbb::parallel_reduce(tbb::blocked_range<T_e>(0, n_particles()),
                    // initial answer
                    std::make_pair(std::numeric_limits<T_e>::max(), 0),
                    // reducing each chunk
                    [&](const tbb::blocked_range<T_e>& r, std::pair<T_e, int> init) -> std::pair<T_e, int>{
                        for (int i=r.begin(); i!=r.end(); i++){
                            if (particles_best_[i] < init.first)
                                init = std::make_pair(particles_best_[i], i);
                        }
                        return init;
                    },
                    // reducing the result of chunks
                    [](std::pair<T_e, int> x, std::pair<T_e, int> y) -> std::pair<T_e, int> {
                        if(x.first < y.first)
                            return x;
                        return y; 
                    });
        return best;
    }
    
};

}; // end of zagros namespace
}; // end of rocky namespace
#endif