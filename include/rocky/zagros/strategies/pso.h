#ifndef ROCKY_ZAGROS_PSO_STRATEGY
#define ROCKY_ZAGROS_PSO_STRATEGY
#include <rocky/zagros/strategies/strategy.h>

namespace rocky{
namespace zagros{
/**
 * @brief Base class for Tribes PSO
 * 
 */
template<typename T_e, int T_dim>
class basic_pso: public search_strategy<T_e, T_dim>{
public:
    typedef Eigen::Map<Eigen::Matrix<T_e, 1, T_dim, Eigen::RowMajor>> eigen_particle;
    enum update_mode {use_particles, use_groups};

protected:
    // system
    system<T_e, T_dim>* problem_;
    // main container
    basic_scontainer<T_e, T_dim>* main_container_;
    // particles velocity
    basic_scontainer<T_e, T_dim>* particles_v_;
    // best solutions for each particle
    basic_scontainer<T_e, T_dim>* particles_best_;
    // groups best solution
    basic_scontainer<T_e, T_dim>* groups_best_;
    // best solution in the current node
    basic_scontainer<T_e, T_dim>* node_best_;
    // best current cluster solution
    basic_scontainer<T_e, T_dim>* cluster_best;  

    // intertial
    T_e hyper_w_;

public:
    basic_pso(system<T_e, T_dim>* problem,
              basic_scontainer<T_e, T_dim>* main_container,
              basic_scontainer<T_e, T_dim>* particles_v,
              basic_scontainer<T_e, T_dim>* particles_best,
              basic_scontainer<T_e, T_dim>* groups_best,
              basic_scontainer<T_e, T_dim>* node_best,
              basic_scontainer<T_e, T_dim>* cluster_best){
        this->problem_ = problem; 
        this->main_container_ = main_container;
        this->particles_v_ = particles_v;
        this->particles_best_ = particles_best;
        this->groups_best_ = groups_best;
        this->node_best_ = node_best;
        this->cluster_best = cluster_best;
    }
    T_e rand_uniform(){
        static std::uniform_real_distribution<T_e> dist(0.0, 1.0);
        return dist(rocky::utils::random::prng);
    }
    // initialize particles velocity to zero
    virtual void initialize_velocity(){
        for(int p=0; p<main_container_->n_particles(); ++p){
            std::fill(particles_v_->particles[p].begin(), particles_v_->particles[p].end(), 0.0);
        }
        spdlog::info("velocity has been initialized");        
    }
    // [todo] evaluate and update best solution of each particle in parallel
    virtual void update_particles_best(int rng_start, int rng_end){
        tbb::parallel_for(rng_start, rng_end, [this](int p){
            T_e obj = this->problem_->objective(this->main_container_->particle(p));
            
            if (obj < this->particles_best_->values[p]){
                this->particles_best_->values[p] = obj;
                // copy the particle solution
                std::copy(this->main_container_->particle(p),
                          this->main_container_->particle(p) + T_dim,
                          this->particles_best_->particle(p));
               
            }
        });
    }
    virtual void update_particles_best(int rng_start=0){
        update_particles_best(rng_start, main_container_->n_particles());
    }
    // update best groups solutions
    virtual void update_groups_best(int rng_start, int rng_end){
        tbb::parallel_for(rng_start, rng_end, [this](int t){
            auto rng = this->main_container_->group_range(t);
            auto min_el = std::min_element(this->particles_best_->value(rng.first),
                                           this->particles_best_->value(rng.second));
            // [todo] we shouldn't copy the solution! 
            if (*min_el < this->groups_best_->values[t]){
                int min_el_ind = static_cast<int>(min_el - this->particles_best_->value(0));
                this->groups_best_->values[t] = *min_el;
                std::copy(this->particles_best_->particle(min_el_ind),
                          this->particles_best_->particle(min_el_ind)+T_dim,
                          this->groups_best_->particle(t));
            }

        });
    }
    virtual void update_groups_best(){
        update_groups_best(0, this->main_container_->n_groups());
    }
    template<update_mode T_um=use_groups>
    void update_node_best(){
        if constexpr(T_um == use_groups){
            auto min_el = std::min_element(this->groups_best_->values.begin(),
                                           this->groups_best_->values.end());
            if (*min_el < this->node_best_->values[0]){
                int min_el_ind = static_cast<int>(min_el - this->groups_best_->values.begin());
                std::copy(this->groups_best_->particle(min_el_ind),
                          this->groups_best_->particle(min_el_ind)+T_dim,
                          this->node_best_->particle(0));
                node_best_->values[0] = *min_el;
            }
        }
    }
    /**
     * @brief update particles velocity in parallel
     * 
     * @return ** void 
     */
    virtual void update_particles_v() = 0;
    /**
     * @brief update particles position in parallel
     * 
     * @return ** void 
     */
    virtual void update_particles_x(){
        tbb::parallel_for(0, this->main_container_->n_particles(), [this](int p){
            eigen_particle x(this->main_container_->particle(p));
            eigen_particle v(this->particles_v_->particle(p));
            x += v;
        });
    }
};


template<typename T_e, int T_dim>
class pso_L1_strategy: public basic_pso<T_e, T_dim>{

typedef Eigen::Map<Eigen::Matrix<T_e, 1, T_dim, Eigen::RowMajor>> eigen_particle;
protected:
    virtual void update_particles_v(){
        tbb::parallel_for(0, this->main_container_->n_particles(), [this](int p){
            int p_group = this->main_container_->particle_group(p);
            eigen_particle x(this->main_container_->particle(p));
            eigen_particle v(this->particles_v_->particle(p));
            eigen_particle p_best(this->particles_best_->particle(p));
            eigen_particle p_best_gr(this->groups_best_->particle(p_group));
            v = v * this->hyper_w_ + (2.0 * this->rand_uniform() * (p_best - x)) 
                                    + (2.0 * this->rand_uniform() * (p_best_gr - x));          
        });
    }
public:
    pso_L1_strategy(system<T_e, T_dim>* problem,
              basic_scontainer<T_e, T_dim>* main_container,
              basic_scontainer<T_e, T_dim>* particles_v,
              basic_scontainer<T_e, T_dim>* particles_best,
              basic_scontainer<T_e, T_dim>* groups_best,
              basic_scontainer<T_e, T_dim>* node_best,
              basic_scontainer<T_e, T_dim>* cluster_best):basic_pso<T_e, T_dim>(problem, main_container, particles_v, particles_best, groups_best, node_best, cluster_best){            
    }
    virtual void apply(){
        this->hyper_w_ = this->rand_uniform();
        this->update_particles_best();
        this->update_groups_best();
        this->update_node_best();
        this->update_particles_v();
        this->update_particles_x();
    }
}; // End of PSO L1


template<typename T_e, int T_dim>
class pso_L2_strategy: public basic_pso<T_e, T_dim>{

typedef Eigen::Map<Eigen::Matrix<T_e, 1, T_dim, Eigen::RowMajor>> eigen_particle;
protected:
    virtual void update_particles_v(){
        tbb::parallel_for(0, this->main_container_->n_particles(), [this](int p){
            int p_group = this->main_container_->particle_group(p);
            eigen_particle x(this->main_container_->particle(p));
            eigen_particle v(this->particles_v_->particle(p));
            eigen_particle p_best(this->particles_best_->particle(p));
            eigen_particle p_best_gr(this->groups_best_->particle(p_group));
            eigen_particle p_best_n(this->node_best_->particle(0));
            if(this->particles_best_->values[p] == this->groups_best_->values[p_group]){
                v = v * this->hyper_w_ + (2.0 * this->rand_uniform() * (p_best - x)) 
                                        + (2.0 * this->rand_uniform() * (p_best_n - x));
            }else{
                v = v * this->hyper_w_ + (2.0 * this->rand_uniform() * (p_best - x)) 
                                        + (2.0 * this->rand_uniform() * (p_best_gr - x));
            }              
        });
    }
public:
    pso_L2_strategy(system<T_e, T_dim>* problem,
              basic_scontainer<T_e, T_dim>* main_container,
              basic_scontainer<T_e, T_dim>* particles_v,
              basic_scontainer<T_e, T_dim>* particles_best,
              basic_scontainer<T_e, T_dim>* groups_best,
              basic_scontainer<T_e, T_dim>* node_best,
              basic_scontainer<T_e, T_dim>* cluster_best):basic_pso<T_e, T_dim>(problem, main_container, particles_v, particles_best, groups_best, node_best, cluster_best){            
    }
    virtual void apply(){
        this->hyper_w_ = this->rand_uniform();
        this->update_particles_best();
        this->update_groups_best();
        this->update_node_best();
        this->update_particles_v();
        this->update_particles_x();
    }
}; // End of PSO L2

};
};


#endif