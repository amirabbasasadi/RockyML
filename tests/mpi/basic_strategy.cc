#define ROCKY_USE_MPI
#include <random>
#include <functional>
#include <algorithm>
#include <chrono>
#include <thread>

#include <tbb/tbb.h>

#include <rocky/zagros/system.h>
#include <rocky/zagros/strategies/init.h>
#include <rocky/zagros/strategies/pso.h>
#include <rocky/zagros/strategies/genetic.h>
#include <rocky/zagros/strategies/differential_evolution.h>
#include <rocky/zagros/strategies/eda.h>
#include <rocky/zagros/strategies/communication.h>
#include <rocky/zagros/benchmark.h>
#include <rocky/utils.h>

int main(int argc, char* argv[]){
    using namespace rocky;
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    typedef float container_type;
    const unsigned dim = 500;
    unsigned n_particles = 500;
    unsigned group_size = 10;
    

    auto problem = std::make_unique<zagros::benchmark::rastrigin<container_type>>(dim, 1.0);
    
    // define a container that contains the solutions
    auto main_container = std::make_unique<zagros::basic_scontainer<container_type, dim>>(n_particles, group_size);
    main_container->allocate();
    // allocate memory for PSO strategy
    spdlog::info("allocating memory for particle swarm");
    auto particles_best = std::make_unique<zagros::basic_scontainer<container_type, dim>>(n_particles, group_size);
    particles_best->allocate();
    auto particles_v = std::make_unique<zagros::basic_scontainer<container_type, dim>>(n_particles, group_size);
    particles_v->allocate();
    auto groups_best = std::make_unique<zagros::basic_scontainer<container_type, dim>>(main_container->n_groups(), 1);
    groups_best->allocate();
    auto node_best = std::make_unique<zagros::basic_scontainer<container_type, dim>>(1, 1);
    node_best->allocate();
    auto cluster_best = std::make_unique<zagros::basic_scontainer<container_type, dim>>(1, 1);
    cluster_best->allocate();
    // define a container that contains the generated candidates for eda
    auto eda_container = std::make_unique<zagros::basic_scontainer<container_type, dim>>(50, group_size);
    eda_container->allocate();
    auto gen_container = std::make_unique<zagros::basic_scontainer<container_type, dim>>(n_particles, group_size);
    gen_container->allocate();
    zagros::pso_l1_strategy<container_type, dim> pso_str_l1(problem.get(),
                                                   main_container.get(),
                                                   particles_v.get(),
                                                   particles_best.get(),
                                                   groups_best.get(),
                                                   node_best.get(),
                                                   cluster_best.get());

    zagros::pso_l2_strategy<container_type, dim> pso_str_l2(problem.get(),
                                                   main_container.get(),
                                                   particles_v.get(),
                                                   particles_best.get(),
                                                   groups_best.get(),
                                                   node_best.get(),
                                                   cluster_best.get());
    
    zagros::pso_l3_strategy<container_type, dim> pso_str_l3(problem.get(),
                                                   main_container.get(),
                                                   particles_v.get(),
                                                   particles_best.get(),
                                                   groups_best.get(),
                                                   node_best.get(),
                                                   cluster_best.get());

    zagros::uniform_init_strategy<container_type, dim> init_str(problem.get(), main_container.get());
   
    zagros::gaussian_mutation<container_type, dim> gaussian_str(problem.get(), particles_best.get(), gen_container.get(), 1, 1.0, 0.5);

    zagros::multipoint_crossover<container_type, dim> crossover_str(problem.get(), particles_best.get(), gen_container.get(), 4);

    zagros::eda_mutivariate_normal<container_type, dim> eda_str(problem.get(), particles_best.get(), eda_container.get(), 50, 50);
    zagros::static_segment_crossover<container_type, dim> seg_str(problem.get(), particles_best.get(), eda_container.get(), 25, 10);

    pso_str_l1.initialize_velocity();
    
    spdlog::info("applying initialization strategy");
    init_str.apply();
    pso_str_l1.apply();

    spdlog::info("applying pso strategy");
    for(int i=0; i<500; i++){
        pso_str_l1.apply();
        seg_str.apply();
        // for(int j=0; j<16; j++)
        //     crossover_str.apply();
        spdlog::info("P({}) L1 iteration {} best node solution is {}", rank, i, *std::min_element(particles_best->values.begin(), particles_best->values.end()));     
    }
    // for(int i=0; i<30; i++){
    //     pso_str_l3.apply();
    //     for(int j=0; j<16; j++)
    //         crossover_str.apply();
    //     spdlog::info("P({}) L3 iteration {} best node solution is {}", rank, i, *std::min_element(particles_best->values.begin(), particles_best->values.end()));     
    // }
    MPI_Finalize();
    return 0;
}

