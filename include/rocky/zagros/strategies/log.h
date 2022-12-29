#ifndef ROCKY_ZAGROS_LOG_STRATEGY
#define ROCKY_ZAGROS_LOG_STRATEGY
#include <rocky/zagros/strategies/strategy.h>

#include<fstream>
#include<sstream>

namespace rocky{
namespace zagros{

/**
 * @brief Interface for logging strategies
 * 
 */
template<typename T_e, int T_dim>
class logging_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief Uniform initializer
 * 
 */
template<typename T_e, int T_dim>
class log_best_strategy: public logging_strategy<T_e, T_dim>{
protected:
    system<T_e, T_dim>* problem_;
    basic_scontainer<T_e, T_dim>* container_;
    std::fstream log_output_;

public:
    log_best_strategy(system<T_e, T_dim>* problem, basic_scontainer<T_e, T_dim>* container){
        this->problem_ = problem;
        this->container_ = container;
        // initialize the output file
        // this->log_output_.open(filename, std::fstream::out);
        
    }
    virtual ~log_best_strategy(){
        if(this->log_output_.is_open())
            this->log_output_.close();
    }
    virtual void apply(){
        // find the best solution
        T_e best = *std::min_element(container_->values.begin(), container_->values.end());
        spdlog::info("best solution : {}", best);
    };
};


};
};
#endif