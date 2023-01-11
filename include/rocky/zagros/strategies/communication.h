#ifndef ROCKY_ZAGROS_COMM_STRATEGY
#define ROCKY_ZAGROS_COMM_STRATEGY

#ifdef ROCKY_USE_MPI
#include<mpi.h>
#endif

#include <rocky/zagros/strategies/strategy.h>
#include<nlohmann/json.hpp>
#include<cpr/cpr.h>



namespace rocky{
namespace zagros{
/**
 * @brief Interface for communication strategies
 * 
 */
template<typename T_e, int T_dim>
class comm_strategy: public basic_strategy<T_e, T_dim>{
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
    void set_target_container(basic_scontainer<T_e, T_dim>* container){
        this->set_target_container = container;
    }
    sync_broadcast_best(basic_scontainer<T_e, T_dim>* container){
        this->cluster_best_container_ = container;
        this->fetch_mpi_info();
    }
    sync_broadcast_best(){
        this->cluster_best_container_ = nullptr;
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
 * @brief A Communication strategy for broadcasting bcd mask
 * 
 */
template<typename T_e, int T_dim>
class sync_bcd_mask: public mpi_strategy<T_e, T_dim>{
protected:
    int* bcd_mask_;
public:
    sync_bcd_mask(int* bcd_mask){
        this->bcd_mask_ = bcd_mask;
        this->fetch_mpi_info();
    }
    virtual void apply(){
        MPI_Bcast(this->bcd_mask_, T_dim, MPI_INT, 0, MPI_COMM_WORLD);
    }
};
#endif

};
};
#endif