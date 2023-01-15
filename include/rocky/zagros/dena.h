#ifndef ROCKY_ZAGROS_DENA_GUARD
#define ROCKY_ZAGROS_DENA_GUARD

#include<map>
#include<stack>
#include<type_traits>
#include<variant>


namespace rocky{
namespace zagros{

// overload pattern for simplifying visitors
template<class... T_s> struct overload : T_s... { using T_s::operator()...; };
template<class... T_s> overload(T_s...) -> overload<T_s...>;

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

struct init_node: public flow_node{};
struct init_uniform: public init_node{
    std::string id;
};
struct init_normal: public init_node{};

struct log_node: public flow_node{};
struct log_best_node: public log_node{
    std::string id;
    local_optimization_log* handler;
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

struct bcd_node: public flow_node{};
enum bcd_mask_generator { uniform };
struct bcd_mask_node: public bcd_node{
    bcd_mask_generator generator;
};

struct run_node: public flow_node{
    std::vector<int> sub_procedure;
};
struct until_convergence: public run_node{};
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
typedef std::variant<log_best_node,
                    bcd_mask_node,
                    comm_cluster_prop_best,
                    init_uniform,
                    init_normal,
                    container_create_node,
                    pso_memory_create_node,
                    pso_group_level_step_node,
                    pso_cluster_level_step_node,
                    run_with_probability_node,
                    run_n_times_node,
                    run_every_n_steps_node> flow_node_variant;

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
std::vector<flow_node_variant> node::nodes = std::vector<flow_node_variant>();
std::map<int, int> node::next_node = std::map<int, int>();

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
        log_best_node node;
        node.id = id;
        node.handler = &handler;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
    // an overload of the best function which selects the best continer automatically
    static flow best(local_optimization_log& handler){
        flow f;
        log_best_node node;
        node.id = std::string("");
        node.handler = &handler;
        auto node_tag = node::register_node<>(node);
        f.procedure.push_back(node_tag);
        return f;
    }
}; // end of local
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
class group{
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
 * @brief group level particle convergence
 * 
 */
class cluster{
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


}; // end of dena
}; // end of zagros
}; // end of rocky
#endif