#define ROCKY_USE_MPI
#include <mpi.h>
#include <catch2/catch_all.hpp>


int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);

    int result = Catch::Session().run(argc, argv);

    MPI_Finalize();
    return result;
}