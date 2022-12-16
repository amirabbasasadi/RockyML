#ifndef ROCKY_ZAGROS_GENETIC_STRATEGY
#define ROCKY_ZAGROS_GENETIC_STRATEGY
#include <rocky/zagros/strategies/strategy.h>

namespace rocky{
namespace zagros{

/**
 * @brief Base class for genetic mutations
 * 
 */
template<typename T_e, int T_dim>
class mutation_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief apply a mutation to k dimensions
 * 
 */
template<typename T_e, int T_dim>
class dimension_tweak_strategy: public mutation_strategy<T_e, T_dim>{
protected:
    // system
    system<T_e, T_dim>* problem_;
    // main container
    basic_scontainer<T_e, T_dim>* target_container_;
    // number of affected dimensions
    int k_;
public:
    T_e sample_dim(){
        std::uniform_int_distribution<> dist(0, T_dim-1);
        return dist(rocky::utils::random::prng);
    }
    T_e sample_particle(int start_rng, int end_rng){
        static std::uniform_int_distribution<> dist(start_rng, end_rng);
        return dist(rocky::utils::random::prng);
    }    
    dimension_tweak_strategy(system<T_e, T_dim>* problem, basic_scontainer<T_e, T_dim>* tgt_container, int k){
        this->problem_ = problem;
        this->target_container_ = tgt_container;
        this->k_ = k;
    }
    virtual void tweak(int p_ind, int dim) = 0;
    virtual void apply(){
        int n_groups = this->target_container_->n_groups();
        tbb::parallel_for(0, n_groups, 1, [this](int gr){
            int dim, p_ind;
            auto group_rng = this->target_container_->group_range(gr);
            // sample a particle
            p_ind = this->sample_particle(group_rng.first, group_rng.second-1);
            // save a backup from affected dims
            auto solution_backup_dims = std::make_unique<int[]>(k_);
            auto solution_backup_vals = std::make_unique<T_e[]>(k_);
            // apply mutation on k dimensions
            for(int d=0; d<k_; d++){
                // choose a random dim
                dim = this->sample_dim();
                solution_backup_dims[d] = dim;
                solution_backup_vals[d] = target_container_->particles[p_ind][dim];
                // apply the mutation on the dimension
                this->tweak(p_ind, dim);
            }
            // check if the mutation caused any improvement
            T_e obj_val = problem_->objective(target_container_->particle(p_ind));
            if(obj_val < target_container_->values[p_ind])
                target_container_->values[p_ind] = obj_val;
            else
                for(int d=0; d<k_; d++)
                    target_container_->particles[p_ind][solution_backup_dims[d]] = solution_backup_vals[d];
                    
        });
        
    }
}; 


/**
 * @brief Gaussian mutation
 * 
 */
template<typename T_e, int T_dim>
class gaussian_mutation: public dimension_tweak_strategy<T_e, T_dim>{
protected:
    T_e mu_;
    T_e sigma_;
public:
    gaussian_mutation(system<T_e, T_dim>* problem, basic_scontainer<T_e, T_dim>* tgt_container, int k=1, T_e mu=0.0, T_e sigma=0.5)
    :dimension_tweak_strategy<T_e, T_dim>(problem, tgt_container, k){
        this->mu_ = mu;
        this->sigma_ = sigma;
    }
    // generate gaussian noise
    T_e gaussian_noise(){
        static std::normal_distribution<T_e> dist(mu_, sigma_);
        auto z = dist(rocky::utils::random::prng);
        return z;
    }
    virtual void tweak(int p_ind, int dim){
        this->target_container_->particles[p_ind][dim] += gaussian_noise();
    }
};

};
};
#endif