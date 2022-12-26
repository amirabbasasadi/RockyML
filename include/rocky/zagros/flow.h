#ifndef ROCKY_ZAGROS_FLOW_GUARD
#define ROCKY_ZAGROS_FLOW_GUARD

#include<map>
#include<stack>
#include<deque>
#include<type_traits>
#include<variant>

#include<rocky/zagros/system.h>
#include<rocky/zagros/strategies/strategy.h>
#include<rocky/zagros/strategies/init.h>
#include <rocky/utils.h>


namespace rocky{
namespace zagros{



namespace flow{
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

struct run_node;
struct until_convergence;
struct run_every_n_steps_node;
struct run_n_times_node;

// a variant containing all nodes
typedef std::variant<null_node,
                    init_uniform,
                    init_normal,
                    container_create_node,
                    run_n_times_node> flow_node_variant;



struct run_node: public flow_node{
    std::deque<flow_node_variant> sub_procedure;
};
struct until_convergence: public run_node{};
struct run_every_n_steps_node: public run_node{};
struct run_n_times_node: public run_node{
    int n_iters;
};


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
        container_create_node node;
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
        init_uniform node;
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
        run_n_times_node node;
        node.n_iters = iters;
        node.sub_procedure.insert(node.sub_procedure.begin(), wrapped_flow.procedure.begin(), wrapped_flow.procedure.end());
        // std::copy(wrapped_flow.procedure.begin(), wrapped_flow.procedure.end(), node.sub_procedure.begin());
        f.procedure.push_front(node);
        return f;
    }
}; // end of init


}; // end of flow


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
    // number of nodes
    int n_nodes;
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
    }
};

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

    void operator()(flow::null_node node){
        spdlog::warn("a null node was found in the stack");
    }
    void operator()(flow::init_uniform node){
        spdlog::info("visiting an init uniform node");
    }
    void operator()(flow::init_normal node){
        
    }
    void operator()(flow::run_n_times_node node){
        spdlog::info("visiting a run {} times node", node.n_iters);
        path_stack->push(node.sub_procedure.begin());
    }
    void operator()(flow::container_create_node node){
        spdlog::info("visiting a container creator");
        main_storage->allocate_container(node.id, node.n_particles, node.group_size);
    }
    
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

    void operator()(flow::null_node node){}
    void operator()(flow::init_uniform node){
        // get the target container
        auto target_cnt = main_storage->container(node.id);
        // reserve the strategy
        auto str = std::make_unique<uniform_init_strategy<T_e, T_dim>>(problem, target_cnt);
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(str));
    }
    void operator()(flow::init_normal node){}
    void operator()(flow::run_n_times_node node){}
    void operator()(flow::container_create_node node){}
    
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

    void operator()(flow::null_node node){}
    void operator()(flow::init_uniform node){
        for(auto& str: main_storage->str_storage[node.tag]){
            str->apply();
            spdlog::info("initialized container");
        }
    }
    void operator()(flow::init_normal node){}
    void operator()(flow::run_n_times_node node){}
    void operator()(flow::container_create_node node){}
    
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
        storage.n_nodes = 0;
    }
    void run(const flow::flow& fl){
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
    void traverse_allocate(const flow::flow& fl){
        auto it = fl.procedure.begin();
        // storage for the traversed path
        std::stack<decltype(it)> path;
        // visitor
        allocation_visitor<T_e, T_dim, decltype(it)> visitor {problem, &storage, &path};
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
            storage.n_nodes++;
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
    void traverse_assign(const flow::flow& fl){
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
    void traverse_run(const flow::flow& fl){
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