#ifndef ROCKY_ZAGROS_FLOW_GUARD
#define ROCKY_ZAGROS_FLOW_GUARD


#include<rocky/zagros/strategies/init.h>
#include<rocky/zagros/strategies/log.h>
#include<rocky/zagros/strategies/pso.h>
#include<rocky/zagros/strategies/blocked_descent.h>
#include<rocky/zagros/dena.h>



namespace rocky{
namespace zagros{

/**
 * @brief runtime storage
 * 
 */
template<typename T_e, int T_dim, int T_block_dim>
class runtime_storage{
public:
    // allocated containers
    std::vector<std::unique_ptr<basic_scontainer<T_e, T_block_dim>>> cnt_storage;
    // allocated strategies
    std::map<int, std::vector<std::unique_ptr<basic_strategy<T_e, T_block_dim>>>> str_storage;
    // mapping the containers' id to their storage blocks
    std::map<std::string, int> cnt_map;
    // a mask representing active variables in blocked descent
    std::vector<int> bcd_mask;
    // state of blocked systems
    std::unique_ptr<basic_scontainer<T_e, T_dim>> blocked_state;
    tbb::enumerable_thread_specific<std::vector<T_e>> th_blocked_states;
    // a temp buffer for broadcasting best partial solution
    std::unique_ptr<basic_scontainer<T_e, T_block_dim>> partial_best;
    // amount of allocated memory
    size_t container_space(){
        size_t allocated_mem = 0;
        for(auto const& cnt: cnt_storage)
            allocated_mem += cnt->space();
        if constexpr(T_dim != T_block_dim){
            allocated_mem += blocked_state->space();
            allocated_mem += partial_best->space();
            for(auto const& th_state: th_blocked_states)
                allocated_mem += th_state.size() * sizeof(T_e);
        }
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
    basic_scontainer<T_e, T_block_dim>* container(std::string id){
        if (!container_exist(id)){
            spdlog::warn("container {} does not exist", id);
        }
        // find the target container
        int target_cnt_ind = cnt_map[id];
        return cnt_storage[target_cnt_ind].get();
    }
    void allocate_container(std::string id, int n_particles, int group_size){
         // allocate a continer
        auto cnt = std::make_unique<basic_scontainer<T_e, T_block_dim>>(n_particles, group_size);
        cnt->allocate();
        cnt_storage.push_back(std::move(cnt));
        // register the id in the storage
        cnt_map[id] = cnt_storage.size()-1;
        spdlog::info("container {} was allocated", id);
    }
    // find the best partial solution in the storage
    void update_partial_best(){
        std::pair<T_e, int> best;
        int best_ci = -1;
        best.first = std::numeric_limits<T_e>::max();
        for(int ci=0; ci<cnt_storage.size(); ci++){
            auto min_and_index = cnt_storage[ci]->best_min_index();
            if((min_and_index.first < best.first) || best_ci == -1){
                best = min_and_index;
                best_ci = ci;
            }        
        }
        if (best.first < partial_best->values[0]){
            // copy the best solution to the container
            std::copy(cnt_storage[best_ci]->particle(best.second),
                    cnt_storage[best_ci]->particle(best.second)+T_block_dim,
                    partial_best->particle(0));
            
            // copy the corresponding min value
            partial_best->values[0] = best.first;
        }
    }
    // synchronize best partial solution
    void sync_partial_best(){
        // Assumption : update_partial_best has been called already
        // This function must be called before regenerating BCD mask
        sync_broadcast_best<T_e, T_block_dim> sync_best_partial_str(partial_best.get());
        spdlog::info("Broadcasting best partial solution...");
        sync_best_partial_str.apply();
        // replace the old partial solution in solution states
        for(int i=0; i<T_block_dim; i++)
            blocked_state->particles[0][bcd_mask[i]] = partial_best->particles[0][i];
        // replace the old partial solution in thread-specific states
        int i=0;
        for(auto& th_state: th_blocked_states){
            th_state[bcd_mask[i]] = partial_best->particles[0][i];
            i++;
        }
    }
    // reset all solution containers
    void reset(){
        for(auto& cnt: cnt_storage)
            cnt->reset_values();
        for(auto& [tag, str_vec]: str_storage)
            for(auto& str: str_vec)
                str->reset();
    }
};



/**
 * @brief Allocation visitor
 * a visitor for traversing the flow and allocating required memory
 */
template<typename T_e, int T_dim, int T_block_dim>
struct allocation_visitor{
    // objective system
    system<T_e>* problem;
    // visitor will change main storage
    runtime_storage<T_e, T_dim, T_block_dim>* main_storage;
    // visitor may also change the path stack in the case of composable flows
    std::stack<int>* path_stack;

    void operator()(dena::bcd_mask_node node){}
    void operator()(dena::log_best_node node){}
    void operator()(dena::comm_cluster_prop_best node){}
    void operator()(dena::init_uniform node){}
    void operator()(dena::init_normal node){}
    void operator()(dena::run_n_times_node node){
        path_stack->push(node.sub_procedure.front());
    }
    void operator()(dena::run_with_probability_node node){
        path_stack->push(node.sub_procedure.front());
    }
    void operator()(dena::container_create_node node){
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
    void operator()(dena::pso_cluster_level_step_node node){}
};
/**
 * @brief Assigning visitor
 * a visitor for linking allocated strategies to the suitable solution container
 */
template<typename T_e, int T_dim, int T_block_dim>
struct assigning_visitor{
    // objective system
    system<T_e>* problem;
    // visitor will change main storage
    runtime_storage<T_e, T_dim, T_block_dim>* main_storage;
    // visitor may also change the path stack in the case of composable flows
    std::stack<int>* path_stack;
    
    void operator()(dena::bcd_mask_node node){
        // reserve the mask generation strategy
        auto gen_str = std::make_unique<bcd_mask_uniform_random<T_e, T_block_dim>>(dynamic_cast<blocked_system<T_e>*>(problem), &(main_storage->bcd_mask));
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(gen_str));
        // reserve the mask synchronization strategy
        auto sync_str = std::make_unique<sync_bcd_mask<T_e, T_block_dim>>(main_storage->bcd_mask.data());
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(sync_str));
    }

    void operator()(dena::log_best_node node){
        basic_scontainer<T_e, T_block_dim>* target_cnt = nullptr;
        if(node.id.size() == 0)
            target_cnt = main_storage->partial_best.get();
        else
            target_cnt = main_storage->container(node.id);
        // reserve the strategy
        
        auto str = std::make_unique<local_log_best<T_e, T_block_dim>>(problem, target_cnt, node.filename);
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(str));
    }
    void operator()(dena::comm_cluster_prop_best node){
        // retrieve the target container
        auto target_cnt = main_storage->container(node.id);
        // create and configure the strategy
        auto str = std::make_unique<sync_broadcast_best<T_e, T_block_dim>>(target_cnt);
        // register the strategy
        main_storage->str_storage[node.tag].push_back(std::move(str));
    }
    void operator()(dena::init_uniform node){
        // get the target container
        auto target_cnt = main_storage->container(node.id);
        // reserve the strategy
        auto str = std::make_unique<uniform_init_strategy<T_e, T_block_dim>>(problem, target_cnt);
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(str));
    }
    void operator()(dena::init_normal node){}
    void operator()(dena::run_n_times_node node){
        path_stack->push(node.sub_procedure.front());
    }
    void operator()(dena::run_with_probability_node node){
        path_stack->push(node.sub_procedure.front());
    }
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
        auto str = std::make_unique<pso_l1_strategy<T_e, T_block_dim>>(problem, main_cnt, particles_v, particles_mem, groups_mem, node_mem, cluster_mem);
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(str));
    }
    void operator()(dena::pso_cluster_level_step_node node){
        // get memory containers
        using namespace dena;
        auto main_cnt = main_storage->container(node.main_cnt_id);
        auto particles_v = main_storage->container(pso::memory::particles_vel(node.memory_id));
        auto particles_mem = main_storage->container(pso::memory::particles_mem(node.memory_id));
        auto groups_mem = main_storage->container(pso::memory::groups_mem(node.memory_id));
        auto node_mem = main_storage->container(pso::memory::node_mem(node.memory_id));
        auto cluster_mem = main_storage->container(pso::memory::cluster_mem(node.memory_id));
        auto str = std::make_unique<pso_l3_strategy<T_e, T_block_dim>>(problem, main_cnt, particles_v, particles_mem, groups_mem, node_mem, cluster_mem);
        // add the strategy to the container
        main_storage->str_storage[node.tag].push_back(std::move(str));
    }
};

/**
 * @brief Running visitor
 * a visitor for traversing and running the flow
 */
template<typename T_e, int T_dim, int T_block_dim, typename T_traverse_fn>
struct running_visitor{
    // objective system
    system<T_e>* problem;
    // visitor will change main storage
    runtime_storage<T_e, T_dim, T_block_dim>* main_storage;
    // visitor may also change the path stack in the case of composable flows
    T_traverse_fn* traverse_fn;

    template<typename T_n>
    void operator()(T_n node){
        if constexpr (std::is_base_of<dena::run_node, T_n>::value){
            // run n times
            if constexpr (std::is_base_of<dena::run_n_times_node, T_n>::value){
                for(int i=0; i<node.n_iters; i++){
                    (*traverse_fn)(node.sub_procedure.front(), problem, main_storage);
                }
            }
            // run with a probability
            if constexpr (std::is_base_of<dena::run_with_probability_node, T_n>::value){
                float sample = utils::random::uniform<float>(0.0, 1.0);
                if(sample < node.prob){
                    (*traverse_fn)(node.sub_procedure.front(), problem, main_storage);
                }
            }
            return;
        }
        if constexpr (std::is_base_of<dena::bcd_mask_node, T_n>::value){
            spdlog::info("running a bcd mask gen node...");
            // synchronize best values for the current state over the cluster
            main_storage->update_partial_best();
            main_storage->sync_partial_best();
            spdlog::info("bcd mask has been generated and synchronized.");
            // generate a new mask
            main_storage->str_storage[node.tag][0]->apply();
            // synchronize the generated mask
            main_storage->str_storage[node.tag][1]->apply();
            // reset all solution containers
            main_storage->reset();
            return;
        }
        if (main_storage->str_storage.find(node.tag) != main_storage->str_storage.end()){
            for(auto& str: main_storage->str_storage[node.tag]){
                str->apply();
                main_storage->update_partial_best();
            }
        }
        
       
    }
};

/**
 * @brief base class for all runtimes
 * 
 */
template<typename T_e, int T_dim, int T_block_dim=T_dim>
class basic_runtime{
public:
    // objective system
    system<T_e>* problem;
    std::unique_ptr<blocked_system<T_e>> blocked_problem;
    runtime_storage<T_e, T_dim, T_block_dim> storage;

    // check if the objective system is blocked 
    constexpr bool blocked(){
        return T_dim != T_block_dim;
    }
    // get problem
    system<T_e>* get_problem(){
        if(blocked())
            return blocked_problem.get();
        else
            return problem;
    }
    basic_runtime(system<T_e>* problem){
        this->problem = problem;
        if (blocked()){
            storage.bcd_mask.resize(T_block_dim);
            std::iota(storage.bcd_mask.begin(), storage.bcd_mask.end(), 0);
            this->blocked_problem = std::make_unique<blocked_system<T_e>>(problem, T_dim, T_block_dim, storage.bcd_mask.data());
            sync_bcd_mask<T_e, T_block_dim> sync_mask_str(storage.bcd_mask.data());
            spdlog::info("broadcasting bcd mask...");
            sync_mask_str.apply();
            // initialize and sync the bcd state
            storage.blocked_state = std::make_unique<basic_scontainer<T_e, T_dim>>(1, 1);
            storage.blocked_state->allocate();
            storage.partial_best = std::make_unique<basic_scontainer<T_e, T_block_dim>>(1, 1);
            storage.partial_best->allocate();
            uniform_init_strategy<T_e, T_dim> init_bcd_state(problem, storage.blocked_state.get());
            init_bcd_state.apply();
            // briadcast the initialized solution
            spdlog::info("broadcasting initial BCD solution state...");
            sync_broadcast_best<T_e, T_dim> sync_bcd_state_str(storage.blocked_state.get());
            sync_bcd_state_str.apply();
            storage.th_blocked_states = tbb::enumerable_thread_specific<std::vector<T_e>>(storage.blocked_state->particles[0]);
            this->blocked_problem->set_solution_state(&(storage.th_blocked_states));

            // storage.update_partial_best();
            // storage.sync_partial_best();
            
        } 
    }
    void run(const dena::flow& fl){
        // allocate memory for running the flow
        this->traverse_allocate(fl);
        spdlog::info("allocation finished");
        // allocate and assign strategies
        this->traverse_assign(fl);
        spdlog::info("assignment finished");
        // run the flow recursively
        this->traverse_run(fl);
    }
    /**
     * @brief allocate required memory for running the flow
     * 
     * @param fl flow
     * @return * void 
     */
    void traverse_allocate(const dena::flow& fl){
        auto it = fl.procedure.front();
        // storage for the traversed path
        std::stack<int> path;
        // visitor
        allocation_visitor<T_e, T_dim, T_block_dim> alloc_visitor {get_problem(), &storage, &path}; 
        // initialize the path
        path.push(it);
        // iterate until there is no node in the stack
        while(!path.empty()){
            it = path.top();
            // remove the visited node
            path.pop();
            auto current = dena::node::nodes[it];
            // push the next node into the stack if it's not null
            it = dena::node::next(it);
            if(it > -1)
                path.push(it);  
            // visit the node
            std::visit(alloc_visitor, current);      
        }  
    }
    /**
     * @brief allocating strategies and assigning them to nodes
     * 
     * @param fl flow
     * @return * void 
     */
    void traverse_assign(const dena::flow& fl){
        auto it = fl.procedure.front();
        // storage for the traversed path
        std::stack<int> path;
        // visitor
        assigning_visitor<T_e, T_dim, T_block_dim> assign_visitor {get_problem(), &storage, &path}; 
        // initialize the path
        path.push(it);
        // iterate until there is no node in the stack
        while(!path.empty()){
            it = path.top();
            // remove the visited node
            path.pop();
            auto current = dena::node::nodes[it];
            // push the next node into the stack if it's not null
            it = dena::node::next(it);
            if(it > -1)
                path.push(it);  
            // visit the node
            std::visit(assign_visitor, current);      
        }  
    }
    static void traverse_run_rec(int root, system<T_e>* problem, runtime_storage<T_e, T_dim, T_block_dim>* storage){
        if(root == -1)
            return;
        // visitor
        running_visitor<T_e, T_dim, T_block_dim, decltype(traverse_run_rec)> run_visitor {problem, storage, &traverse_run_rec}; 
        // iterate until there is no node left in the stack
        auto node = dena::node::nodes[root];
        std::visit(run_visitor, node);
        traverse_run_rec(dena::node::next(root), problem, storage);
    }
    /**
     * @brief unnning the flow
     * 
     * @param fl flow
     * @return * void 
     */
    void traverse_run(const dena::flow& fl){
        // running the flow and sub-flows recursively
        this->traverse_run_rec(fl.procedure.front(), get_problem(), &storage);
    }
};


}; // end of zagros
}; // end of rocky
#endif