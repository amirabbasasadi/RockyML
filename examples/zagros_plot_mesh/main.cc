#include <mpi.h>
#include <rocky/zagros/benchmark.h>
#include <rocky/zagros/flow.h>

using namespace rocky;
using namespace rocky::zagros;
using namespace rocky::zagros::dena;

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    
    // define the optimization problems
    const int dim = 2;
    const int block_dim = 2;
    const int width = 500;

    std::vector<std::string> names;
    std::vector<std::unique_ptr<zagros::system<float>>> functions;
    
    names.push_back("ackley");
    functions.push_back(std::move(std::make_unique<benchmark::ackley<float>>(dim)));
    names.push_back("rastrigin");
    functions.push_back(std::move(std::make_unique<benchmark::rastrigin<float>>(dim)));
    names.push_back("griewank");
    functions.push_back(std::move(std::make_unique<benchmark::griewank<float>>(dim)));
    names.push_back("dropwave");
    functions.push_back(std::move(std::make_unique<benchmark::dropwave<float>>(dim)));
    
    for(int i=0; i<names.size(); i++){
        spdlog::info("generating mesh for {} ...", names[i]);
        auto optimizer = analyze::plot::heatmap(names[i], width);
        basic_runtime<float, dim, block_dim> runtime(functions[i].get());
        runtime.run(optimizer);
    }

    MPI_Finalize();
    return 0;
}