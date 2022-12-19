#ifndef ROCKY_ZAGROS_COMM_STRATEGY
#define ROCKY_ZAGROS_COMM_STRATEGY

#include <rocky/zagros/strategies/strategy.h>

#ifdef ROCKY_USE_MPI
#include<mpi.h>
#endif

namespace rocky{
namespace zagros{
/**
 * @brief Interface for communication strategies
 * 
 */
template<typename T_e, int T_dim>
class comm_strategy: basic_strategy<T_e, T_dim>{
public:
    virtual void apply() = 0;
};


#ifdef ROCKY_USE_MPI
/**
 * @brief base class for all strategies who need MPI communication
 * 
 */
template<typename T_e, int T_dim>
class mpi_strategy: public comm_strategy<T_e, T_dim>{
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

/**
 * @brief A Communication strategy for broadcasting best solution
 * 
 */
template<typename T_e, int T_dim>
class sync_broadcast_best: public mpi_strategy<T_e, T_dim>{
protected:
    basic_scontainer<T_e, T_dim>* cluster_best_container_;
public:
    sync_broadcast_best(basic_scontainer<T_e, T_dim>* container){
        this->cluster_best_container_ = container;
        this->fetch_mpi_info();
    }
    virtual void apply(){
        // identify the process
        int rank = this->mpi_rank();
        // ask everyone in the cluster to find the min value
        struct {
            T_e cluster_best_min;
            int rank;
        } data_out, result;

        data_out.cluster_best_min = cluster_best_container_->values[0];
        data_out.rank = rank;

        if constexpr(std::is_same<T_e, double>::value){
            MPI_Allreduce(&data_out, &result, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
            MPI_Bcast(cluster_best_container_->particle(0), T_dim, MPI_DOUBLE, result.rank, MPI_COMM_WORLD);
        }
        if constexpr(std::is_same<T_e, float>::value){
            MPI_Allreduce(&data_out, &result, 1, MPI_FLOAT_INT, MPI_MINLOC, MPI_COMM_WORLD);
            MPI_Bcast(cluster_best_container_->particle(0), T_dim, MPI_FLOAT, result.rank, MPI_COMM_WORLD);
        } 
        cluster_best_container_->values[0] = result.cluster_best_min;
    }
};

/**
 * @brief A communication strategy for sharing best solution using RMA passive target model
 * 
 */
template<typename T_e, int T_dim>
class rma_share_best: public mpi_strategy<T_e, T_dim>{
protected:
    // MPI windows
    MPI_Win argmin_window_;
    MPI_Win min_window_;
    MPI_Win lock_window_;

    basic_scontainer<T_e, T_dim>* cluster_best_container_;
    basic_scontainer<T_e, T_dim>* temp_container_;
    basic_scontainer<T_e, T_dim>* window_cnt_;
    /**
     * @brief lock for local operations
     * for global operations we use MPI_lock
     */
    std::vector<int> lock_; 
public:
    // allocate a window for sharing the solution on all processes
    void allocate_window(){
        // a window for the solution
        MPI_Win_create(window_cnt_->particle(0), T_dim * sizeof(T_e), sizeof(T_e),
                       MPI_INFO_NULL, MPI_COMM_WORLD, &argmin_window_); 
        // a window for the value of the solution
        MPI_Win_create(window_cnt_->value(0), sizeof(T_e), sizeof(T_e),
                       MPI_INFO_NULL, MPI_COMM_WORLD, &min_window_); 
        // a window for locking
        MPI_Win_create(lock_.data(), sizeof(int), sizeof(int),
                       MPI_INFO_NULL, MPI_COMM_WORLD, &lock_window_);   
    }
    // free the allocated memory
    void free_window(){
        MPI_Win_free(&argmin_window_);
        MPI_Win_free(&min_window_);
        MPI_Win_free(&lock_window_);
    }
    int random_process(int n_procs){
        static std::uniform_int_distribution<> dist(0, n_procs-1);
        return dist(rocky::utils::random::prng);
    }
    virtual void apply(){
        // identify the process
        int rank = this->mpi_rank();
        int n_procs = this->mpi_num_procs();
        // choose a random target process
        int target_ranks = rank; 
        while(target_ranks == rank)
            target_ranks = this->random_process(n_procs);
       spdlog::info("P({}) chooses P({}) as a target", rank, target_rank);
        // check to see if the target process has a better solution
        MPI_Get(temp_container_->value(0), 1, MPI_FLOAT, target_rank, 0, 1, MPI_FLOAT, min_window);
        
        spdg::info("P({}) gets min value from P({}) : ", rank, target_rank, temp_container_->values[0]);
        if(temp_container_->values[0] < cluster_best_container_->values[0]){
            int locked = 1;
            // notify the target process
            MPI_Put(&locked, 1, MPI_INT, target_rank, rank, 1, MPI_INT, lock_window_);
            // get the solution
            MPI_Get(cluster_best_container_->value(0), 1, MPI_FLOAT, target_rank, 0, T_dim, MPI_FLOAT, argmin_window);
            locked = 0;
            // notify the target process
            MPI_Put(&locked, 1, MPI_INT, target_rank, rank, 1, MPI_INT, lock_window_);
        }

    }
};



/**
 * @brief An async communication strategy for broadcasting best solution
 * 
 */
template<typename T_e, int T_dim>
class async_broadcast_best: public mpi_strategy<T_e, T_dim>{
protected:
    // handlers to ckeck the status of message
    MPI_Request reduction_req_;
    MPI_Request broadcast_req_;

    basic_scontainer<T_e, T_dim>* cluster_best_container_;
public:
    async_broadcast_best(basic_scontainer<T_e, T_dim>* container){
        this->cluster_best_container_ = container;
        this->fetch_mpi_info();
    }
    virtual void apply(){
        // identify the process
        int rank = this->mpi_rank();
        // ask everyone in the cluster to find the min value
        struct {
            T_e cluster_best_min;
            int rank;
        } data_out, result;

        data_out.cluster_best_min = cluster_best_container_->values[0];
        data_out.rank = rank;

        if constexpr(std::is_same<T_e, double>::value){
            MPI_Iallreduce(&data_out, &result, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD, &reduction_req_);
            MPI_Ibcast(cluster_best_container_->particle(0), T_dim, MPI_DOUBLE, result.rank, MPI_COMM_WORLD, &broadcast_req_);
        }
        if constexpr(std::is_same<T_e, float>::value){
            MPI_Iallreduce(&data_out, &result, 1, MPI_FLOAT_INT, MPI_MINLOC, MPI_COMM_WORLD, &reduction_req_);
            spdlog::info("reduction is done. the min process is {} and min value is {}", result.rank, result.cluster_best_min);
            MPI_Ibcast(cluster_best_container_->particle(0), T_dim, MPI_FLOAT, result.rank, MPI_COMM_WORLD, &broadcast_req_);
        } 
        cluster_best_container_->values[0] = result.cluster_best_min;
    }
};


#endif

};
};
#endif