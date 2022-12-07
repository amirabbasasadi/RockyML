#ifndef ROCKY_ZAGROS_PSO_GUARD
#define ROCKY_ZAGROS_PSO_GUARD
#include<iostream>
#include<cmath>
#include<utility>
#include<memory>
#include<limits>
#include<random>
#include<mpi.h>
#include<tbb/tbb.h>
#include<chrono>
#include<Eigen/Core>
#include<rocky/zagros/distributed.h>
#include<rocky/zagros/system.h>
#include<rocky/zagros/strategy.h>
#include<rocky/zagros/benchmark.h>
#include<rocky/zagros/logging.h>
namespace rocky{
namespace zagros{



/**
 * @brief A Unified Class for Population-based algorithms that need MPI
 * Generally based on TribePSO
 * Reference : Chen, K., Li, T. and Cao, T., 2006. Tribe-PSO: A novel global optimization algorithm and its application in molecular docking. Chemometrics and intelligent laboratory systems, 82(1-2), pp.248-259.
 * 
 * We devide the particles on each node into `L` Tribes
 * The first optimization layer is parallelized at Thread-level using TBB
 * The last optimization layer is parallelized at Process-level using MPI
 * 
 * Each tribe contains Pt particles where Pt = N_particles / L 
 * Particle p belongs to tribe t if floor(p/Pt) = t 
 * It's beneficial to keep the particles of a each Tribe close to each other in the memory
 * 
 * By setting L to 1 the algorithm is equivalent to classic PSO
 * 
 */
template<typename T_e, int T_dim, int T_n_particles, int T_n_tribes>
class swarm_mpi:  public basic_mpi_optimizer, public optimization_log{
protected:
    // target system for optimization
    zagros::system<T_e, T_dim>* problem_;
    // different optimization phases
    enum phase { phase_I, phase_II, phase_III, phase_IV };
    // particles position
    T_e* particles_x_;
    // particles best solution
    T_e* particles_best_min_;
    T_e* particles_best_argmin_;
    // best current solution of each tribe
    T_e* tribes_best_min_;
    T_e** tribes_best_argmin_;   
    // particles velocity
    T_e* particles_v_;
    // best current global solution
    T_e global_best_min_;
    T_e* global_best_argmin_;

    // best current cluster solution
    T_e cluster_best_min_;
    T_e* cluster_best_argmin_;

    T_e hyper_w;

public:
    constexpr int n_particles() { return T_n_particles; } 
    constexpr int particles_per_tribe(){
        return T_n_particles / T_n_tribes;
    }
    int tribe(int particle_ind){
        return particle_ind / particles_per_tribe();
    }
    std::pair<int, int> tribe_range(int t){
        int tribe_s = t * particles_per_tribe();
        int tribe_e = tribe_s + particles_per_tribe();
        if(t == T_n_tribes - 1)
            tribe_e = T_n_particles;

        return std::make_pair(tribe_s, tribe_e);            
    }
    T_e random_uniform(){
        static thread_local std::mt19937 generator;
        std::uniform_real_distribution<T_e> distribution(0.0,1.0);
        return distribution(generator);
    }
    T_e random_initial_uniform(int p){
        static thread_local std::mt19937 generator;
        std::uniform_real_distribution<T_e> distribution(this->problem_->lower_bound(p),
                                                         this->problem_->upper_bound(p));
        return distribution(generator);
    }
    /**
     * @brief setting the target system for optimization
     * 
     * @param sys a zagros system to optimize
     * @return ** void 
     */
    void add_system(zagros::system<T_e, T_dim>* sys){
        problem_ = sys;
    }
    /**
     * @brief implementing the log interface
     * adding a header
     * @return ** void 
     */
    void log_on_open() override{
        this->log_output << "time,tribe,value" << "\n";
    }
    /**
     * @brief 
     * tracking the best solution of tribes
     * @return ** void 
     */
    void log_step(int time) override{
        for(int t=0; t<T_n_tribes; t++)
            this->log_output << time << "," << t << "," << tribes_best_min_[t] << "\n";
    }
    // allocate the required memory
    virtual void allocate(){
        // allocate particles memory
        particles_best_min_ = new T_e[T_n_particles];
        particles_best_argmin_ = new T_e[T_n_particles * T_dim];
        particles_x_ = new T_e[T_n_particles * T_dim];
        particles_v_ = new T_e[T_n_particles * T_dim];
         // allocate tribes memory
        tribes_best_min_ = new T_e[T_n_tribes];
        tribes_best_argmin_ = new T_e*[T_n_tribes];
        // best global solution
        global_best_argmin_ = new T_e[T_dim];
        // best cluster solution
        cluster_best_argmin_ = new T_e[T_dim];
    }
    /**
     * @brief initial value for best solutions
     * 
     * @return ** void 
     */
    virtual void initialize_best(){
        std::fill(particles_best_min_, particles_best_min_+ T_n_particles, std::numeric_limits<T_e>::max());
        std::fill(tribes_best_min_, tribes_best_min_+ T_n_tribes, std::numeric_limits<T_e>::max());
        global_best_min_ = std::numeric_limits<T_e>::max();
        cluster_best_min_ = std::numeric_limits<T_e>::max();
    }
    // initialize positions randomly
    // Todo : the system should be able to override this
    virtual void initialize_particles_x(){
        tbb::parallel_for(0, T_n_particles, 1, [this](int p){
            for(int d=0; d<T_dim; d++)
                this->particles_x_[p*T_dim + d] = this->random_initial_uniform(d);
        });
    }
    // initialize particles velocity to zero
    virtual void initialize_particles_v(){
        std::fill(particles_v_, particles_v_ + T_n_particles * T_dim, 0.0);
    }
    // main initialization
    virtual void initialize(){
        initialize_best();
        initialize_particles_x();
        initialize_particles_v();
    }
    // evaluate and update best solution of each particle in parallel
    virtual void update_particles_best(){
        tbb::parallel_for(0, T_n_particles, [this](int p){
            T_e obj = this->problem_->objective(this->particles_x_ + p*T_dim);
            if (obj < this->particles_best_min_[p]){
                this->particles_best_min_[p] = obj;
                // copy the particle solution
                std::copy(this->particles_x_ + p*T_dim,
                          this->particles_x_ + (p+1)*T_dim,
                          this->particles_best_argmin_ + p*T_dim);
            }
        });
    }
    // update best tribes solutions
    virtual void update_tribes_best(){
        tbb::parallel_for(0, T_n_tribes, [this](int t){
            auto t_range = this->tribe_range(t);
            auto min_el = std::min_element(this->particles_best_min_ + t_range.first,
                                           this->particles_best_min_ + t_range.second);
            
            if (*min_el < this->tribes_best_min_[t]){
                int min_el_ind = static_cast<int>(min_el - this->particles_best_min_);
                this->tribes_best_min_[t] = *min_el;
                this->tribes_best_argmin_[t] = this->particles_best_argmin_ + min_el_ind * T_dim;
            }

        });
    }
    /**
     * @brief update particles velocity in parallel
     * 
     * @return ** void 
     */
    template<phase T_phase>
    void update_particles_v(){
        tbb::parallel_for(0, T_n_particles, [this](int p){
            int p_tribe = this->tribe(p);
            Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> x(this->particles_x_ + p*T_dim);
            Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> v(this->particles_v_ + p*T_dim);
            Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> p_best(this->particles_best_argmin_ + p*T_dim);
            Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> p_best_t(this->tribes_best_argmin_[p_tribe]);
            // update the velocity
            if constexpr(T_phase == phase::phase_I){
                v = v * this->hyper_w + (2.0 * this->random_uniform() * (p_best - x)) 
                                      + (2.0 * this->random_uniform() * (p_best_t - x));
            }
            if constexpr(T_phase == phase::phase_II){
                Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> p_best_g(this->global_best_argmin_);
                if(this->particles_best_min_[p] == this->tribes_best_min_[p_tribe]){
                    v = v * this->hyper_w + (2.0 * this->random_uniform() * (p_best - x)) 
                                          + (2.0 * this->random_uniform() * (p_best_g - x));
                }else{
                    v = v * this->hyper_w + (2.0 * this->random_uniform() * (p_best - x)) 
                                          + (2.0 * this->random_uniform() * (p_best_t - x));
                }         
            }
            if constexpr(T_phase == phase::phase_III){
                Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> p_best_g(this->global_best_argmin_);
                 v = v * this->hyper_w + (2.0 * this->random_uniform() * (p_best - x)) 
                                       + (2.0 * this->random_uniform() * (p_best_g - x));
            }
            if constexpr(T_phase == phase::phase_IV){
                Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> p_best_c(this->cluster_best_argmin_);
                 v = v * this->hyper_w + (2.0 * this->random_uniform() * (p_best - x)) 
                                       + (2.0 * this->random_uniform() * (p_best_c - x));
            }
        });
    }
    // 
    /**
     * @brief update particles position in parallel
     * 
     * @return ** void 
     */
    virtual void update_particles_x(){
        tbb::parallel_for(0, T_n_particles, [this](int p){
            int p_tribe = this->tribe(p);
            Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> x(this->particles_x_ + p*T_dim);
            Eigen::Map<Eigen::Matrix<T_e, 1, T_dim>> v(this->particles_v_ + p*T_dim);
            x += v;
        });
    }

    /**
     * @brief find best current solution sequentially
     * 
     * @return void
     */
    void update_global_best(){
        auto min_el = std::min_element(tribes_best_min_, tribes_best_min_ + T_n_tribes);
        int min_el_ind = static_cast<int>(min_el - tribes_best_min_);
        global_best_min_ = *min_el;
        std::copy(tribes_best_argmin_[min_el_ind],
                  tribes_best_argmin_[min_el_ind] + T_dim,
                  global_best_argmin_);
    }
    /**
     * @brief update cluster solution using message passing
     * 
     * @return ** void 
     */
    void update_cluster_best(){
        // identify the process
        int rank = this->mpi_rank();
        // ask everyone in the cluster to find the min value
        struct {
            T_e cluster_best_min;
            int rank;
        } data_out, result;

        data_out.cluster_best_min = global_best_min_;
        data_out.rank = rank;

        if constexpr(std::is_same<T_e, double>::value){
            MPI_Allreduce(&data_out, &result, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
        }
        if constexpr(std::is_same<T_e, float>::value){
            MPI_Allreduce(&data_out, &result, 1, MPI_FLOAT_INT, MPI_MINLOC, MPI_COMM_WORLD);
        }
        
        // the owner of best solution should broadcast its solution to the others
        if(rank == result.rank){
            std::copy(global_best_argmin_,
                      global_best_argmin_ + T_dim,
                      cluster_best_argmin_);
        }
        if constexpr(std::is_same<T_e, double>::value){
            MPI_Bcast(cluster_best_argmin_, T_dim, MPI_DOUBLE, result.rank, MPI_COMM_WORLD);
        }
        if constexpr(std::is_same<T_e, float>::value){
            MPI_Bcast(cluster_best_argmin_, T_dim, MPI_FLOAT, result.rank, MPI_COMM_WORLD);
        }   
        cluster_best_min_ = result.cluster_best_min;
    }
    /**
     * @brief find best current solution in parallel
     * Warning : This is fast though not memory efficient so should not be used for large number of particles
     * 
     * @return void
     */
   std::pair<T_e, int> find_global_best_parallel(){
        std::pair<T_e, int> best = tbb::parallel_reduce(tbb::blocked_range<T_e>(0, T_n_particles),
                    // initial answer
                    std::make_pair(std::numeric_limits<T_e>::max(), 0),
                    // reducing each chunk
                    [this](const tbb::blocked_range<T_e>& r, std::pair<T_e, int> init) -> std::pair<T_e, int>{
                        for (int i=r.begin(); i!=r.end(); i++){
                            if (this->particles_best_min_[i] < init.first)
                                init = std::make_pair(this->particles_best_min_[i], i);
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
    virtual void iter(int iters){

    }
    // release the reserved memory
    virtual ~swarm_mpi(){
        delete[] particles_x_;
        delete[] particles_v_;
        delete[] particles_best_min_;
        delete[] particles_best_argmin_;    
        delete[] tribes_best_min_;
        delete[] tribes_best_argmin_;  
        delete[] global_best_argmin_;
        delete[] cluster_best_argmin_;
    }

};

}; // end of zagros namespace
}; // end of rocky namespace
#endif