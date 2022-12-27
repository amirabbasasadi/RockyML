#ifndef ROCKY_ZAGROS_FLOW_GUARD
#define ROCKY_ZAGROS_FLOW_GUARD

#include<deque>
#include<map>
#include<stack>
#include<type_traits>
#include<variant>

#include<rocky/zagros/system.h>
#include<rocky/zagros/strategies/init.h>
#include<rocky/zagros/strategies/pso.h>
#include<rocky/zagros/strategies/strategy.h>
#include <rocky/utils.h>


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

struct init_node: public flow_node{};
struct init_uniform: public init_node{
    std::string id;
};
struct init_normal: public init_node{};

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
struct pso_node_level_step_node: public pso_step_node{};

struct run_node;
struct until_convergence;
struct run_every_n_steps_node;
struct run_n_times_node;

// a variant containing all nodes
typedef std::variant<null_node,
                    init_uniform,
                    init_normal,
                    container_create_node,
                    pso_memory_create_node,
                    pso_group_level_step_node,
                    run_n_times_node> flow_node_variant;



struct run_node: public flow_node{
    std::deque<flow_node_variant> sub_procedure;
};
struct until_convergence: public run_node{};
struct run_every_n_steps_node: public run_node{};
struct run_n_times_node: public run_node{
    int n_iters;
};


class node{
public:
    static int n_nodes;
    template<typename T_n>
    static T_n create(){ 
        T_n node;
        node.tag = n_nodes++;
        return node;
    } 
};
int node::n_nodes = 0;

class flow{
public:
    std::deque<flow_node_variant> procedure;
    size_t total_memory;

    flow(){
        null_node node;
        procedure.push_front(node);
    }

    flow& operator >>(const flow& f){
        auto node = *(f.procedure.begin());
        auto last_it = std::prev(this->procedure.end());
        if((*last_it).index() == 0)
            this->procedure.erase(last_it);
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
        auto node = node::create<container_create_node>();
        node.id = id;
        node.n_particles = n_particles;
        node.group_size = group_size;
        f.procedure.push_front(node);
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
        auto node = node::create<init_uniform>();
        node.id = id;
        f.procedure.push_front(node);
        return f;
    }
}; // end of init

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
        auto node = node::create<run_n_times_node>();
        node.n_iters = iters;
        node.sub_procedure.insert(node.sub_procedure.begin(), wrapped_flow.procedure.begin(), wrapped_flow.procedure.end());
        f.procedure.push_front(node);
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
        auto node = node::create<pso_memory_create_node>();
        node.memory_id = mem_id;
        node.main_cnt_id = main_id;
        f.procedure.push_front(node);
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
class group_level{
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
        auto node = node::create<pso_group_level_step_node>();
        node.memory_id = mem_id;
        node.main_cnt_id = main_id;
        f.procedure.push_front(node);
        return f;
    }
}; // end of group level

}; // end of pso


}; // end of dena


/**
 * @brief runtime storage
 * 
 */
template<typename T_e, int T_dim>
class runtime_storage{
public:
    // allocated containers
    std::vector<std::unique_ptr<basic_scontainer<T_e, T_dim>>> cnt_storage;
    // allocated strategies
    std::map<int, std::vector<std::unique_ptr<basic_strategy<T_e, T_dim>>>> str_storage;
    // mapping the containers' id to their storage blocks
    std::map<std::string, int> cnt_map;
    // amount of allocated memory
    size_t container_space(){
        size_t allocated_mem = 0;
        for(auto const& cnt: cnt_storage)
            allocated_mem += cnt->space();
        return allocated_mem;
    }
    /**
     * @brief check if a container exists in the memory
     * 
     * @param id container id
     */
    bool container_exist(std::string id){
        auto it = cnt_map.find(id);
        return it != cnt_map.end();
    }
    /**
     * @brief access to specific container
     * 
     * @param id 
     * @return * basic_scontainer<T_e, T_dim>* 
     */
    basic_scontainer<T_e, T_dim>* container(std::string id){
        if (!container_exist(id)){
            spdlog::warn("container {} does not exist", id);
        }
        // find the target container
        int target_cnt_ind = cnt_map[id];
        return cnt_storage[target_cnt_ind].get();
    }
    void allocate_container(std::string id, int n_particles, int group_size){
         // allocate a continer
        auto cnt = std::make_unique<basic_scontainer<T_e, T_dim>>(n_particles, group_size);
        cnt->allocate();
        cnt_storage.push_back(std::move(cnt));
        // register the id in the storage
        cnt_map[id] = cnt_storage.size()-1;
        spdlog::info("container {} was allocated", id);
    }
};

// overload pattern for simplifying visitors
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

/**
 * @brief Allocation visitor
 * a visitor for traversing the flow and allocating required memory
 */
template<typename T_e, int T_dim, typename T_stack_e>
struct allocation_visitor{
    // objective system
    system<T_e, T_dim>* problem;
    // visitor will change main storage
    runtime_storage<T_e, T_dim>* main_storage;
    // visitor may also change the path stack in the case of composable flows
    std::stack<T_stack_e>* path_stack;

    void operator()(dena::null_node node){
        spdlog::warn("a null node was found in the stack");
    }
    void operator()(dena::init_uniform node){
        spdlog::info("visiting an init uniform node");
    }
    void operator()(dena::init_normal node){
        
    }
    void operator()(dena::run_n_times_node node){
        spdlog::info("visiting a run {} times node", node.n_iters);
        path_stack->push(node.sub_procedure.begin());
    }
    void operator()(dena::container_create_node node){
        spdlog::info("visiting a container creator");
        main_storage->allocate_container(node.id, node.n_particles, node.group_size);
    }
    void operator()(dena::pso_memory_create_node node){
        // allocate required solution containers for particle swarm
        auto main_cnt = main_storage->container(node.main_cnt_id);
        int n_particles = main_cnt->n_particles();
        int n_groups = main_cnt->n_groups();
        int group_size = main_cnt->group_size();
        
        using namespace dena;
        // particles velocity
        main_storage->allocate_container(pso::memory::particles_vel(node.memory_id), n_particles, group_size);
        // particles memory
        main_storage->allocate_container(pso::memory::particles_mem(node.memory_id), n_particles, group_size);
        // groups memory
        main_storage->allocate_container(pso::memory::groups_mem(node.memory_id), n_groups, 1);
        // node memory
        main_storage->allocate_container(pso::memory::node_mem(node.memory_id), 1, 1);
        // cluster memory
        main_storage->allocate_container(pso::memory::cluster_mem(node.memory_id), 1, 1);
    }
    void operator()(dena::pso_group_level_step_node node){}
};
/**
 * @brief Assigning visitor
 * a visitor for linking allocated strategies to the suitable solution container
 */
template<typename T_e, int T_dim, typename T_stack_e>
struct assigning_visitor{
    // objective system
    system<T_e, T_dim>* problem;
    // visitor will change main storage
    runtime_storage<T_e, T_dim>* main_storage;
    // visitor may also change the path stack in the case of composable flows
    std::stack<T_stack_e>* path_stack;

    void operator()(dena::null_node node){}
    void operator()(dena::init_uniform node){
        // get the target container
        auto target_cnt = main_storage->container(node.id);
        // reserve the strategy
        auto str = std::make_unique<uniform_init_strategy<T_e, T_dim>>(problem, target_cnt);
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(str));
    }
    void operator()(dena::init_normal node){}
    void operator()(dena::run_n_times_node node){}
    void operator()(dena::container_create_node node){}
    void operator()(dena::pso_memory_create_node node){}
    void operator()(dena::pso_group_level_step_node node){
        // get memory containers
        using namespace dena;
        auto main_cnt = main_storage->container(node.main_cnt_id);
        auto particles_v = main_storage->container(pso::memory::particles_vel(node.memory_id));
        auto particles_mem = main_storage->container(pso::memory::particles_mem(node.memory_id));
        auto groups_mem = main_storage->container(pso::memory::groups_mem(node.memory_id));
        auto node_mem = main_storage->container(pso::memory::node_mem(node.memory_id));
        auto cluster_mem = main_storage->container(pso::memory::cluster_mem(node.memory_id));
        auto str = std::make_unique<pso_l1_strategy<T_e, T_dim>>(problem, main_cnt, particles_v, particles_mem, groups_mem, node_mem, cluster_mem);
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(str));
    }
    
};

/**
 * @brief Running visitor
 * a visitor for traversing and running the flow
 */
template<typename T_e, int T_dim, typename T_stack_e>
struct running_visitor{
    // objective system
    system<T_e, T_dim>* problem;
    // visitor will change main storage
    runtime_storage<T_e, T_dim>* main_storage;
    // visitor may also change the path stack in the case of composable flows
    std::stack<T_stack_e>* path_stack;

    template<typename T_n>
    void operator()(T_n node){
        if (main_storage->str_storage.find(node.tag) != main_storage->str_storage.end()){
            for(auto& str: main_storage->str_storage[node.tag]){
                str->apply();
            }
        }
    }
};

/**
 * @brief base class for all runtimes
 * 
 */
template<typename T_e, int T_dim>
class basic_runtime{
public:
    // objective system
    system<T_e, T_dim>* problem;
    runtime_storage<T_e, T_dim> storage;

    basic_runtime(system<T_e, T_dim>* problem){
        this->problem = problem;
    }
    void run(const dena::flow& fl){
        // allocate memory for running the flow
        this->traverse_allocate(fl);
        // allocate and assign strategies
        this->traverse_assign(fl);
        // running the flow
        this->traverse_run(fl);
    }
    /**
     * @brief allocate required memory for running the flow
     * 
     * @param fl flow
     * @return * void 
     */
    void traverse_allocate(const dena::flow& fl){
        auto it = fl.procedure.begin();
        // storage for the traversed path
        std::stack<decltype(it)> path;
        // visitor
        allocation_visitor<T_e, T_dim, decltype(it)> alloc_visitor {problem, &storage, &path}; 
        // initialize the path
        path.push(it);
        // iterate until there is no node in the stack
        while(!path.empty()){
            it = path.top();
            // remove the visited node
            path.pop();
            auto current = *it;
            // visit the node
            std::visit(alloc_visitor, current);
            // push the next node into the stack if it's not null
            it = std::next(it);
            if((*it).index() != 0)
                path.push(it);        
        }  
    }
    /**
     * @brief allocating strategies and assigning them to nodes
     * 
     * @param fl flow
     * @return * void 
     */
    void traverse_assign(const dena::flow& fl){
        auto it = fl.procedure.begin();
        // storage for the traversed path
        std::stack<decltype(it)> path;
        // visitor
        assigning_visitor<T_e, T_dim, decltype(it)> visitor {problem, &storage, &path};
        // initialize the path
        path.push(it);
        // iterate until there is no node in the stack
        while(!path.empty()){
            it = path.top();
            // remove the visited node
            path.pop();
            auto current = *it;
            // visit the node
            std::visit(visitor, current);
            // push the next node into the stack if it's not null
            it = std::next(it);
            if((*it).index() != 0)
                path.push(it);        
        }  
    }
    /**
     * @brief unnning the flow
     * 
     * @param fl flow
     * @return * void 
     */
    void traverse_run(const dena::flow& fl){
        auto it = fl.procedure.begin();
        // storage for the traversed path
        std::stack<decltype(it)> path;
        // visitor
        running_visitor<T_e, T_dim, decltype(it)> visitor {problem, &storage, &path};
        // initialize the path
        path.push(it);
        // iterate until there is no node in the stack
        while(!path.empty()){
            it = path.top();
            // remove the visited node
            path.pop();
            auto current = *it;
            // visit the node
            std::visit(visitor, current);
            // push the next node into the stack if it's not null
            it = std::next(it);
            if((*it).index() != 0)
                path.push(it);        
        }  
    }
};


}; // end of zagros
}; // end of rocky
#endif