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

};
};
#endif