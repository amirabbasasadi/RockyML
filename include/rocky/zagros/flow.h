#ifndef ROCKY_ZAGROS_FLOW_GUARD
#define ROCKY_ZAGROS_FLOW_GUARD

#include<type_traits>

#include<tbb/tbb.h>
#include<Eigen/Core>

#include<rocky/zagros/system.h>
#include<rocky/zagros/strategies/strategy.h>
#include <rocky/utils.h>


namespace rocky{
namespace zagros{

namespace flow{
/**
 * @brief abstract flow node
 * 
 */
struct flow_node{};

struct container_node: public flow_node{};
struct container_create_node: public container_node{
    int n_particles;
    int n_groups;
};


struct run_node: public flow_node{
    std::list<std::shared_ptr<flow_node>> procedure;
};

struct until_convergence: public run_node{};

struct run_every_n_steps_node: public run_node{};
struct run_n_times_node: public run_node{
    int n_iters;
};

struct init_node: public flow_node{};
struct init_uniform: public init_node{};
struct init_normal: public init_node{};


class flow{
public:
    std::list<std::shared_ptr<flow_node>> next;
    size_t total_memory;

public:
    flow& operator >>(const flow& f){
        auto node = *(f.next.begin());
        this->next.insert(this->next.end(), f.next.begin(), f.next.end());
        return *this;
    }
    // Traverse the flow tree and allocate the required memmory
    void allocate(){

    }
};


class container{
public:
    static flow create(int n_particles){
        flow f;
        auto node = std::make_shared<container_create_node>();
        node->n_particles = n_particles;
        f.next.push_back(node);
        return f;
    }
};

class init{
public:
    static flow uniform(){
        flow f;
        auto node = std::make_shared<init_uniform>();
        f.next.push_back(node);
        return f;
    }
}; // end of init

class run{
public:
    static flow n_times(int iters, const flow& wrapped_flow){
        flow f;
        auto node = std::make_shared<run_n_times_node>();
        node->n_iters = iters;
        node->procedure = wrapped_flow.next;
        f.next.push_back(node);
        return f;
    }
}; // end of init


}; // end of flow
}; // end of zagros
}; // end of rocky
#endif