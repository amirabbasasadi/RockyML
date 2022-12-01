#ifndef ROCKY_ZAGROS_DISTRIBUTED_GUARD
#define ROCKY_ZAGROS_DISTRIBUTED_GUARD
#include<iostream>
#include<mpi.h>
#include<memory>

namespace rocky{
namespace zagros{

/**
 * @brief basic class for all mpi optimizers
 * 
 */
class basic_mpi_optimizer{
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

}; // end of zagros namespace
}; // end of rocky namespace
#endif