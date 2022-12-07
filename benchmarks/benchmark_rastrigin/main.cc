#include <iostream>
#include <fstream>
#include <random>
#include <functional>
#include <algorithm>
#include <tbb/tbb.h>
#include <rocky/etna/blocks.h>
#include <rocky/zagros/system.h>
#include <rocky/zagros/distributed.h>
#include <rocky/zagros/swarm.h>
#include <rocky/zagros/benchmark.h>
#include <chrono>
#include <thread>


int main(int argc, char* argv[]){
    using namespace rocky;
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    srand(rank);

    const unsigned n_particles = 200;
    const unsigned dim = 1000;
    const unsigned n_tribes = 10;

    zagros::swarm_mpi<float, dim, n_particles, n_tribes> optimizer;
    auto system = new rocky::zagros::benchmark::rastrigin<float, dim>();

    optimizer.fetch_mpi_info();
    optimizer.add_system(system);
    optimizer.allocate();
    optimizer.initialize();

    int iters = 100;
    if(rank == 0)
        optimizer.log_init("objective.csv");

    for(int i=0; i<iters; i++){
        optimizer.iter();
        if(rank == 0)
            optimizer.log_step(i);
    }
    if(rank == 0)
        optimizer.log_save();

    delete system;
    MPI_Finalize();
    return 0;
}

