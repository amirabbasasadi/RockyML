/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_ZAGROS_DENA_GUARD
#define ROCKY_ZAGROS_DENA_GUARD

#include<vector>
#include<deque>
#include<map>
#include<stack>
#include<type_traits>
#include<variant>

#include<rocky/zagros/strategies/log.h>


namespace rocky{
namespace zagros{

namespace dena{


/**
 * @brief abstract flow node
 * 
 */
struct flow_node{
    // a unique identifier
    int tag;
};

struct null_node: public flow_node{};

struct container_node: public flow_node{};
struct container_create_node: public container_node{
    std::string id;
    int n_particles;
    int group_size;
};
struct container_select_from_node: public container_node{
    std::string des;
    std::string src;
};

struct init_node: public flow_node{};
struct init_uniform: public init_node{
    std::string id;
};
struct init_normal: public init_node{};

struct log_node: public flow_node{};
struct log_local_best_node: public log_node{
    std::string id;
    local_optimization_log* handler;
};
struct log_comet_best_node: public log_node{
    std::string id;
    comet_optimization_log* handler;
};

struct comm_node: public flow_node{};
struct comm_cluster_prop_best: public comm_node{
    std::string id;
};

struct pso_node: public flow_node{};
struct pso_memory_create_node: public pso_node{
    std::string memory_id;
    std::string main_cnt_id;
};
struct pso_step_node: public pso_node{
    std::string memory_id;
    std::string main_cnt_id;
};
struct pso_group_level_step_node: public pso_step_node{};
struct pso_cluster_level_step_node: public pso_step_node{};

struct mutate_node: public flow_node{};
struct mutate_gaussian_node: public mutate_node{
    std::string id;
    int dims;
    float mu;
    float sigma;
};

struct crossover_node: public flow_node{};
struct crossover_multipoint_node: public crossover_node{
    std::string id;
    int dims;
};
struct crossover_differential_evolution_node: public crossover_node{
    std::string id;
    float crossover_prob;
    float differential_weight;
};
struct crossover_segment_node: public crossover_node{
    std::string id;
    int segment_length;
};

struct eda_node: public flow_node{};
struct eda_mvn_node: public eda_node{};
struct eda_mvn_fullcov_node: public eda_mvn_node{
    std::string id;
};

struct analysis_node: public flow_node{};
struct plot_node: public analysis_node{};
struct plot_heatmap_node: public plot_node{
    int width;
    int height;
    std::string label;
};

struct bcd_node: public flow_node{};
enum bcd_mask_generator { uniform };
struct bcd_mask_node: public bcd_node{
    bcd_mask_generator generator;
};

struct run_node: public flow_node{
    std::vector<int> sub_procedure;
};
struct run_until_no_improve_node: public run_node{
    std::string id;
    int max_check;
};
struct run_with_probability_node: public run_node{
    float prob;
};
struct run_n_times_node: public run_node{
    int n_iters;
};
struct run_every_n_steps_node: public run_node{
    int period;
};


// a variant containing all nodes
typedef std::variant<log_local_best_node,
                    log_comet_best_node,
                    bcd_mask_node,
                    comm_cluster_prop_best,
                    init_uniform,
                    init_normal,
                    container_create_node,
                    container_select_from_node,
                    pso_memory_create_node,
                    pso_group_level_step_node,
                    pso_cluster_level_step_node,
                    mutate_gaussian_node,
                    crossover_multipoint_node,
                    crossover_differential_evolution_node,
                    crossover_segment_node,
                    eda_mvn_fullcov_node,
                    plot_heatmap_node,
                    run_with_probability_node,
                    run_n_times_node,
                    run_every_n_steps_node,
                    run_until_no_improve_node> flow_node_variant;

class node{
public:
    static std::vector<flow_node_variant> nodes;
    static std::map<int, int> next_node;
    template<typename T_n>
    static int register_node(T_n node){ 
        node.tag = nodes.size();
        nodes.push_back(node);
        next_node[node.tag] = -1;
        return node.tag;
    }
    static void register_link(int s, int e){
        next_node[s] = e;
    }
    static int next(int tag){
        return next_node[tag];
    }
};


class flow{
public:
    std::vector<int> procedure;
    size_t total_memory;

    flow& operator >>(const flow& f){
        // concat the two procedures
        auto last_node = procedure.back();
        auto first_node = f.procedure.front();
        node::register_link(last_node, first_node);
        this->procedure.insert(this->procedure.end(), f.procedure.begin(), f.procedure.end());
        return *this;
    }
};

/**
 * @brief factories for soultion containers
 * 
 */
class container{
public:
    /**
     * @brief create a solution
     * 
     * @param id std::string container's id
     * @param n_particles int number of particles
     * @param group_size int size of each group
     * @return * flow 
     */
    static flow create(std::string id, int n_particles, int group_size){
        flow f;
        container_create_node node;
        node.id = id;
        node.n_particles = n_particles;
        node.group_size = group_size;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    static flow create(std::string id, int n_particles){
        return create(id, n_particles, n_particles);
    }
    static flow create(std::string id){
        return create(id, 1, 1);
    }
    static flow select_from(std::string des, std::string src){
        flow f;
        container_select_from_node node;
        node.des = des;
        node.src = src;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
};

/**
 * @brief utils for dena
 * 
 */
class utils{
public:
    /**
     * @brief generate the name for temporal containers
     * 
     */
    static std::string temp_name(int tag){
        return fmt::format("__temp__T[{}]__", tag);
    }
    /**
     * @brief generate the name for temporal containers
     * 
     */
    static std::string temp_name(int tag, std::string desc){
        return fmt::format("__temp__{}__{}__", tag, desc);
    }
};

/**
 * @brief factories for initialization strategies
 * 
 */
class init{
public:
    /**
     * @brief initialize particles uniformly
     * 
     * @return * flow 
     */
    static flow uniform(std::string id){
        flow f;
        init_uniform node;
        node.id = id;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
}; // end of init

namespace log{
class local{
public:
    /**
     * @brief local logging strategies
     * 
     * @return * flow 
     */
    static flow best(std::string id, local_optimization_log& handler){
        flow f;
        log_local_best_node node;
        node.id = id;
        node.handler = &handler;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    // an overload of the best function which selects the best continer automatically
    static flow best(local_optimization_log& handler){
        return best("__best__", handler);
    }
}; // end of local

class comet{
public:
    /**
     * @brief local logging strategies
     * 
     * @return * flow 
     */
    static flow best(std::string id, comet_optimization_log& handler){
        flow f;
        log_comet_best_node node;
        node.id = id;
        node.handler = &handler;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    // an overload of the best function which selects the best continer automatically
    static flow best(comet_optimization_log& handler){
        return best(std::string("__best__"), handler);
    }
}; // end of comet

}; // end of log

namespace blocked_descent{
/**
 * @brief factories for uniform BCD strategy * 
 */
class uniform{
public:
    static flow step(){
        flow f;
        bcd_mask_node node;
        node.generator = bcd_mask_generator::uniform;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
}; // end of uniform
}; // end of blocked descent


namespace propagate{
class cluster{
public:
/**
 * @brief propagate the best solution across nodes
 * 
 * @param id target container
 * @return * flow 
 */
static flow best(std::string id){
    flow f;
    comm_cluster_prop_best node;
    node.id = id;
    auto node_tag = node::register_node<>(node);
    f.procedure.push_back(node_tag);
    return f;
}
}; // end of cluster
}; // end of comm

/**
 * @brief factories for composable flows
 * 
 */
class run{
public:
    /**
     * @brief run the wrapped flow `n` times
     * [todo] there is a problem with the last node
     * @param iters int number of iterations
     * @param wrapped_flow target flow for running
     * @return * flow 
     */
    static flow n_times(int iters, const flow& wrapped_flow){
        flow f;
        run_n_times_node node;
        node.n_iters = iters;
        node.sub_procedure.insert(node.sub_procedure.end(), wrapped_flow.procedure.begin(), wrapped_flow.procedure.end());
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    /**
     * @brief run a flow with a specified probability
     * 
     * @param prob the probability of running the flow
     * @param wrapped_flow 
     * @return * flow 
     */
    static flow with_probability(float prob, const flow& wrapped_flow){
        flow f;
        run_with_probability_node node;
        node.prob = prob;
        node.sub_procedure.insert(node.sub_procedure.end(), wrapped_flow.procedure.begin(), wrapped_flow.procedure.end());
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    /**
     * @brief run a flow with given period
     * 
     * @param n the period of running the flow
     * @param wrapped_flow 
     * @return * flow 
     */
    static flow every_n_steps(int n, const flow& wrapped_flow){
        flow f;
        run_every_n_steps_node node;
        node.period = n;
        node.sub_procedure.insert(node.sub_procedure.end(), wrapped_flow.procedure.begin(), wrapped_flow.procedure.end());
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    /**
     * @brief run a flow until the best solution of a container does not improve
     * 
     * @param id the id of the target container
     * @param n maximum number of waiting before termination
     * @param wrapped_flow 
     * @return * flow 
     */
    static flow while_improve(std::string id, int n, const flow& wrapped_flow){
        flow f;
        run_until_no_improve_node node;
        node.id = id;
        node.max_check = n;
        node.sub_procedure.insert(node.sub_procedure.end(), wrapped_flow.procedure.begin(), wrapped_flow.procedure.end());
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    /**
     * @brief run a flow until the best solution does not improve
     * 
     * @param n maximum number of waiting before termination
     * @param wrapped_flow 
     * @return * flow 
     */
    static flow while_improve(int n, const flow& wrapped_flow){
        return while_improve(std::string("__best__"), n, wrapped_flow);
    }
    /**
     * @brief run a flow until the best solution does not improve
     * 
     * @param wrapped_flow 
     * @return * flow 
     */
    static flow while_improve(const flow& wrapped_flow){
        return while_improve(std::string("__best__"), 20, wrapped_flow);
    }

}; // end of init


namespace pso{
/**
 * @brief utilities for manipulating pso memory
 * 
 */
class memory{
public:
    /**
     * @brief creating a node for pso memory allocation
     * 
     * @param mem_id a unique name for refering to memory
     * @param main_id pso target solution container
     * @return * flow 
     */
    static flow create(std::string mem_id, std::string main_id){
        flow f;
        pso_memory_create_node node;
        node.memory_id = mem_id;
        node.main_cnt_id = main_id;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    /**
     * @brief id of particles velocity
     * 
     * @param base memory id
     * @return * std::string 
     */
    static std::string particles_vel(std::string base){
        return base + std::string("__pvel__");
    }
    /**
     * @brief id of particles memory
     * 
     * @param base memory id
     * @return * std::string 
     */
    static std::string particles_mem(std::string base){
        return base + std::string("__pmem__");
    }
    /**
     * @brief id of groups memory
     * 
     * @param base memory id
     * @return * std::string 
     */
    static std::string groups_mem(std::string base){
        return base + std::string("__gmem__");
    }
    /**
     * @brief id of node memory
     * 
     * @param base memory id
     * @return * std::string 
     */
    static std::string node_mem(std::string base){
        return base + std::string("__nmem__");
    }
    /**
     * @brief id of cluster memory
     * 
     * @param base memory id
     * @return * std::string 
     */
    static std::string cluster_mem(std::string base){
        return base + std::string("__cmem__");
    } 
}; // end of memory

/**
 * @brief group level particle convergence
 * 
 */
class local{
public:
    /**
     * @brief single step based on pso-l1
     * 
     * @param mem_id 
     * @param main_id 
     * @return * flow 
     */
    static flow step(std::string mem_id, std::string main_id){
        flow f;
        pso_group_level_step_node node;
        node.memory_id = mem_id;
        node.main_cnt_id = main_id;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
}; // end of group level

/**
 * @brief cluster level particle convergence
 * 
 */
class global{
public:
    /**
     * @brief single step based on pso-l3
     * 
     * @param mem_id 
     * @param main_id 
     * @return * flow 
     */
    static flow step(std::string mem_id, std::string main_id){
        flow f;
        pso_cluster_level_step_node node;
        node.memory_id = mem_id;
        node.main_cnt_id = main_id;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
}; // end of group level

}; // end of pso

class mutate{
public:
    /**
     * @brief Gaussian mutation
     * 
     * @param id target container
     * @param dims number of affected dimensions
     * @param mu mean of the gaussian noise
     * @param sigma std of the gaussian noise
     * @return * flow 
     */
    static flow gaussian(std::string id, int dims=1, float mu=0.0, float sigma=1.0){
        flow f;
        mutate_gaussian_node node;
        node.id = id;
        node.dims = dims;
        node.mu = mu;
        node.sigma = sigma;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }

}; // end of mutate

class crossover{
public:
    /**
     * @brief multipoint crossover
     * 
     * @param id target container
     * @param dims number of affected dimensions
     * @return * flow 
     */
    static flow multipoint(std::string id, int dims=1){
        flow f;
        crossover_multipoint_node node;
        node.id = id;
        node.dims = dims;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    /**
     * @brief differential evolution
     * 
     * @param id target container
     * @param cr crossover probability
     * @param dw differential weight
     * @return * flow 
     */
    static flow differential_evolution(std::string id, float cr=0.9, float dw=0.9){
        flow f;
        crossover_differential_evolution_node node;
        node.id = id;
        node.crossover_prob = cr;
        node.differential_weight = dw;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    static flow segment(std::string id, int segment_length){
        flow f;
        crossover_segment_node node;
        node.id = id;
        node.segment_length = segment_length;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }

}; // end of mutate

/**
 * @brief factories for estimation of distribution algorithms
 * 
 */
namespace eda{
/**
 * @brief mutivariate normal EDA
 * 
 */
class mvn{
public:
    /**
     * @brief Multivariate Normal with Full Covariance Matrix
     * 
     * @param id target container
     * @param sample_size number of samples to estimate covariance matrix
     * @return * flow 
     */
    static flow full_cov(std::string id){
        flow f;
        eda_mvn_fullcov_node node;
        node.id = id;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
}; // end of mvn
}; // end of eda

/**
 * @brief utils for analyzing optimizers and loss functions
 * 
 */
namespace analyze{

/**
 * @brief utilities for plotting loss function or optimization stage
 * 
 */
class plot{
public:
    /**
     * @brief plotting a heatmap of loss function
     * 
     * @param label a string which will be included in the name of output file
     * @param width width of the heatmap in pixels
     * @param height height of the heatmap in pixels
     * @return * flow 
     */
    static flow heatmap(std::string label, int width, int height){
        flow f;
        plot_heatmap_node node;
        node.label = label;
        node.width = width;
        node.height = height;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    static flow heatmap(std::string label, int width){
        return heatmap(label, width, width);
    }
    static flow heatmap(std::string label){
        return heatmap(label, 500);
    }
    static flow heatmap(){
        return heatmap(std::string("heatmap"));
    }
}; // end of plot

}; // end of analysis

}; // end of dena
}; // end of zagros
}; // end of rocky
#endif