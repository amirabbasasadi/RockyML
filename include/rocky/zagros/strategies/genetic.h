#ifndef ROCKY_ZAGROS_GENETIC_STRATEGY
#define ROCKY_ZAGROS_GENETIC_STRATEGY
#include <rocky/zagros/strategies/strategy.h>
#include <set>
#include <iterator>

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
    system<T_e>* problem_;
    // main container
    basic_scontainer<T_e, T_dim>* target_container_;
    // number of affected dimensions
    int k_;
public:
    dimension_tweak_strategy(system<T_e>* problem, basic_scontainer<T_e, T_dim>* tgt_container, int k){
        this->problem_ = problem;
        this->target_container_ = tgt_container;
        this->k_ = k;
    }
    virtual void tweak(int p_ind, int dim) = 0;
    virtual void apply(){
        int n_groups = this->target_container_->n_groups();
        tbb::parallel_for(0, n_groups, 1, [this](int gr){
            int dim, p_ind;
            // sample a particle
            p_ind = this->target_container_->sample_particle(gr);
            // save a backup from affected dims
            auto solution_backup_dims = std::make_unique<int[]>(k_);
            auto solution_backup_vals = std::make_unique<T_e[]>(k_);
            // apply mutation on k dimensions
            for(int d=0; d<k_; d++){
                // choose a random dim
                dim = this->target_container_->sample_dim();
                solution_backup_dims[d] = dim;
                solution_backup_vals[d] = this->target_container_->particles[p_ind][dim];
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
    gaussian_mutation(system<T_e>* problem, basic_scontainer<T_e, T_dim>* tgt_container, int k=1, T_e mu=0.0, T_e sigma=0.5)
    :dimension_tweak_strategy<T_e, T_dim>(problem, tgt_container, k){
        this->mu_ = mu;
        this->sigma_ = sigma;
    }
    // generate gaussian noise
    T_e gaussian_noise(){
        static std::normal_distribution<T_e> dist(mu_, sigma_);
        auto z = dist(rocky::utils::random::prng());
        return z;
    }
    virtual void tweak(int p_ind, int dim){
        this->target_container_->particles[p_ind][dim] += gaussian_noise();
    }
};

/**
 * @brief Base class for genetic crossovers
 * 
 */
template<typename T_e, int T_dim>
class crossover_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief Multipoint crossover
 * 
 */
template<typename T_e, int T_dim>
class multipoint_crossover: public basic_strategy<T_e, T_dim>{
protected:
   // system
    system<T_e, T_dim>* problem_;
    // main container
    basic_scontainer<T_e, T_dim>* container_;
    // maximum number of affected dimensions
    int k_;
public:
    multipoint_crossover(system<T_e>* problem, basic_scontainer<T_e, T_dim>* container, int k){
        this->k_ = k;
        this->problem_ = problem;
        this->container_ = container;
    }
    virtual void apply(){
        tbb::parallel_for(0, container_->n_groups(), [this](int group){
            // choose parents
            auto parents = this->container_->sample_pair(group);
            // affected dims
            std::set<int> dims;
            for(int d=0; d<k_; d++)
                dims.insert(this->container_->sample_dim());
            // apply the crossover
            for(auto dim: dims)
                std::swap(this->container_->particles[parents.first][dim],
                          this->container_->particles[parents.second][dim]);
             // keep the new solution if there was any improvement
            T_e new_val = this->problem_->objective(this->container_->particle(parents.first));
            // restore the previous particle otherwise 
            if (new_val > this->container_->values[parents.first])
                for(auto dim: dims)
                    std::swap(this->container_->particles[parents.first][dim],
                            this->container_->particles[parents.second][dim]);
            else  // replace the min value
                this->container_->values[parents.first] = new_val;
        });
    }
};



};
};
#endif