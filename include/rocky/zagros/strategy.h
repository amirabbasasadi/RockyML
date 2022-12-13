#ifndef ROCKY_ZAGROS_STRATEGY
#define ROCKY_ZAGROS_STRATEGY
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<chrono>

#ifdef ROCKY_USE_MPI
#include<mpi.h>
#endif

#include<tbb/tbb.h>
#include<Eigen/Core>

#include<rocky/zagros/system.h>
#include<rocky/zagros/containers/swarm.h>
#include <rocky/utils.h>


namespace rocky{
namespace zagros{


/**
 * @brief Interface for all strategies
 * 
 */
template<typename T_e, int T_dim>
class basic_strategy{
public:
    virtual void apply(system<T_e, T_dim>* sys, basic_swarm<T_e, T_dim>* main_swarm) = 0;
};

/**
 * @brief Interface for evolution strategies
 * 
 */
template<typename T_e, int T_dim>
class search_strategy: basic_strategy<T_e, T_dim>{
public:
    virtual void apply(system<T_e, T_dim>* sys, basic_swarm<T_e, T_dim>* main_swarm) = 0;
};

/**
 * @brief Interface for initialization strategies
 * 
 */
template<typename T_e, int T_dim>
class init_strategy: basic_strategy<T_e, T_dim>{
public:
    virtual void apply(system<T_e, T_dim>* sys, basic_swarm<T_e, T_dim>* main_swarm) = 0;
};

/**
 * @brief Interface for communication strategies
 * 
 */
template<typename T_e, int T_dim>
class comm_strategy: basic_strategy<T_e, T_dim>{
public:
    virtual void apply(system<T_e, T_dim>* sys, basic_swarm<T_e, T_dim>* main_swarm) = 0;
};

/**
 * @brief Interface for initialization strategies
 * 
 */
template<typename T_e, int T_dim>
class uniform_init_strategy: init_strategy<T_e, T_dim>{
protected:
    T_e rand_uniform(T_e lb, T_e ub){
        std::uniform_real_distribution<T_e> dist(lb, ub);
        return dist(rocky::utils::random::prng);
    }
public:
    virtual void apply(system<T_e, T_dim>* sys, basic_swarm<T_e, T_dim>* swarm){
        tbb::parallel_for(0, swarm->n_particles(), [&](auto p){
            for(int d=0; d<T_dim; ++d)
                swarm->particle(p)[d] = rand_uniform(sys->lower_bound(p), sys->upper_bound(p));
        });
    };
};

#ifdef ROCKY_USE_MPI
/**
 * @brief base class for all strategies who need MPI communication
 * 
 */
template<typename T_e, int T_dim>
class mpi_strategy: public comm_strategy<T_e, T_dim>{
public:
    protected:
     // number of MPI processes
    int mpi_num_procs_;
    // MPI rank
    int mpi_rank_; 
public:
    /**
     * @brief receive MPI information including
     * - number of mpi processes
     * - rank of the current process
     * 
     * @return ** void 
     */
    void fetch_mpi_info(){
        MPI_Comm_size(MPI_COMM_WORLD, &mpi_num_procs_);
        MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank_);
    }

    int mpi_num_procs() const{
        return mpi_num_procs_;
    }
    int mpi_rank() const{
        return mpi_rank_;
    }

};


template<typename T_e, int T_dim>
class broadcast_best_solution: public mpi_strategy<T_e, T_dim>{
    void update_cluster_best(T_e* cluster_best_argmin, T_e* cluster_best_min){
        // identify the process
        int rank = this->mpi_rank();
        // ask everyone in the cluster to find the min value
        struct {
            T_e cluster_best_min;
            int rank;
        } data_out, result;

        data_out.cluster_best_min = *cluster_best_min;
        data_out.rank = rank;

        if constexpr(std::is_same<T_e, double>::value){
            MPI_Allreduce(&data_out, &result, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
            MPI_Bcast(cluster_best_argmin, T_dim, MPI_DOUBLE, result.rank, MPI_COMM_WORLD);
        }
        if constexpr(std::is_same<T_e, float>::value){
            MPI_Allreduce(&data_out, &result, 1, MPI_FLOAT_INT, MPI_MINLOC, MPI_COMM_WORLD);
            MPI_Bcast(cluster_best_argmin, T_dim, MPI_FLOAT, result.rank, MPI_COMM_WORLD);
        } 
        *cluster_best_min = result.cluster_best_min;
    }
};
#endif

template<typename T_e, int T_dim>
class basic_pso: public search_strategy<T_e, T_dim>{
    typedef Eigen::Map<Eigen::Matrix<T_e, 1, T_dim, Eigen::RowMajor>> eigen_particle;
    typedef basic_swarm<T_e, T_dim> swarm;
    
    enum phase {phase_I, phase_II, phase_III, phase_IV};
protected:
    // system
    system<T_e, T_dim>* problem_;
    // main swarm
    swarm* main_swarm_;
    // particles velocity
    std::vector<std::vector<T_e>> particles_v_;
    // best solutions for each particle
    std::unique_ptr<swarm> particles_best_;
    // groups best solution
    std::vector<T_e> groups_best_min_;
    std::vector<T_e*> groups_best_argmin_;
     // best solution in the current node
    T_e node_best_min_;
    T_e* node_best_argmin_;
    // best current cluster solution
    T_e cluster_best_min_;
    std::vector<T_e> cluster_best_argmin_;

    T_e hyper_w;
    

public:
    virtual void init(swarm* main_swarm){
        this->main_swarm_ = main_swarm;
    }
 // allocate the required memory
    virtual void allocate(){
        particles_best_ = std::make_unique<swarm>(main_swarm_->n_particles(), main_swarm_->group_size());
        particles_best_->allocate();
        // allocate particles memory
        particles_v_.resize(main_swarm_->n_particles());
        for(int p=0; p<main_swarm_->n_particles(); ++p)
            particles_v_[p].resize(T_dim);
         // allocate groups memory
        groups_best_min_.resize(main_swarm_->n_groups());
        groups_best_argmin_.resize(main_swarm_->n_groups());
        std::fill(groups_best_min_.begin(), groups_best_min_.end(), std::numeric_limits<T_e>::max());
        // best cluster solution
        cluster_best_argmin_.resize(T_dim);
    }
    T_e rand_uniform(){
        static std::uniform_real_distribution<T_e> dist(0.0, 1.0);
        return dist(rocky::utils::random::prng);
    }
    // initialize particles velocity to zero
    virtual void initialize_velocity(){
        for(int p=0; p<main_swarm_->n_particles(); ++p)
            std::fill(particles_v_[p].begin(), particles_v_[p].end(), 0.0);
    }
    // [todo] evaluate and update best solution of each particle in parallel
    virtual void update_particles_best(int rng_start, int rng_end){
        tbb::parallel_for(rng_start, rng_end, [this](int p){
            T_e obj = this->problem_->objective(this->main_swarm_->particle(p));
            
            if (obj < this->particles_best_->values[p]){
                this->particles_best_->values[p] = obj;
                // copy the particle solution
                std::copy(this->main_swarm_->particle(p),
                          this->main_swarm_->particle(p) + T_dim,
                          this->particles_best_->particle(p));
               
            }
        });
    }
    virtual void update_particles_best(int rng_start=0){
        update_particles_best(rng_start, main_swarm_->n_particles());
    }
    // update best groups solutions
    virtual void update_groups_best(int rng_start, int rng_end){
        tbb::parallel_for(rng_start, rng_end, [this](int t){
            auto rng = this->main_swarm_->group_range(t);
            auto min_el = std::min_element(this->particles_best_->value(rng.first),
                                           this->particles_best_->value(rng.second));
            // update min and argmin for each group 
            if (*min_el < this->groups_best_min_[t]){
                int min_el_ind = static_cast<int>(min_el - this->particles_best_->value(0));
                this->groups_best_min_[t] = *min_el;
                this->groups_best_argmin_[t] = this->particles_best_->particle(min_el_ind);
            }

        });
    }
    virtual void update_groups_best(){
        update_groups_best(0, this->main_swarm_->n_groups());
    }
    /**
     * @brief update particles velocity in parallel
     * 
     * @return ** void 
     */
    template<phase T_phase>
    void update_particles_v(){
        tbb::parallel_for(0, this->main_swarm_->n_particles(), [this](int p){
            int p_group = this->main_swarm_->particle_group(p);
            eigen_particle x(this->main_swarm_->particle(p));
            eigen_particle v(this->particles_v_[p].data());
            eigen_particle p_best(this->particles_best_->particle(p));
            eigen_particle p_best_gr(this->groups_best_argmin_[p_group]);
            
            // update the velocity
            if constexpr(T_phase == phase::phase_I){
                v = v * this->hyper_w + (2.0 * this->rand_uniform() * (p_best - x)) 
                                      + (2.0 * this->rand_uniform() * (p_best_gr - x));
            }
            if constexpr(T_phase == phase::phase_II){
                eigen_particle p_best_n(this->node_best_argmin_);
                if(this->particles_best_min_[p] == this->groups_best_min_[p_group]){
                    v = v * this->hyper_w + (2.0 * this->rand_uniform() * (p_best - x)) 
                                          + (2.0 * this->rand_uniform() * (p_best_n - x));
                }else{
                    v = v * this->hyper_w + (2.0 * this->rand_uniform() * (p_best - x)) 
                                          + (2.0 * this->rand_uniform() * (p_best_gr - x));
                }         
            }
            if constexpr(T_phase == phase::phase_III){
                eigen_particle p_best_n(this->node_best_argmin_);
                 v = v * this->hyper_w + (2.0 * this->rand_uniform() * (p_best - x)) 
                                       + (2.0 * this->rand_uniform() * (p_best_n - x));
            }
            if constexpr(T_phase == phase::phase_IV){
                eigen_particle p_best_c(this->cluster_best_argmin_.data());
                 v = v * this->hyper_w + (2.0 * this->rand_uniform() * (p_best - x)) 
                                       + (2.0 * this->rand_uniform() * (p_best_c - x));
            }
        });
    }
    /**
     * @brief update particles position in parallel
     * 
     * @return ** void 
     */
    virtual void update_particles_x(){
        tbb::parallel_for(0, this->main_swarm_->n_particles(), [this](int p){
            eigen_particle x(this->main_swarm_->particle(p));
            eigen_particle v(this->particles_v_[p].data());
            x += v;
        });
    }
    virtual void apply(system<T_e, T_dim>* sys, basic_swarm<T_e, T_dim>* main_swarm){
        this->hyper_w = rand_uniform();
        this->problem_ = sys;
        this->main_swarm_ = main_swarm;
        this->update_particles_best();
        this->update_groups_best();
        std::cout << "best solution is " << *std::min_element(this->groups_best_min_.begin(), this->groups_best_min_.end()) << std::endl;
        this->update_particles_v<phase_I>();
        this->update_particles_x();
    }
};
};
};


#endif