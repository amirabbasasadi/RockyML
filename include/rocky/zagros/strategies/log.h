#ifndef ROCKY_ZAGROS_LOG_STRATEGY
#define ROCKY_ZAGROS_LOG_STRATEGY
#include <rocky/zagros/strategies/strategy.h>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include<future>
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
 * @brief Log the best solution in the container
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

/**
 * @brief Interface for Comet logging strategies
 * 
 */
template<typename T_e, int T_dim>
class comet_strategy: public logging_strategy<T_e, T_dim>{};

/**
 * @brief Uniform initializer
 * 
 */
template<typename T_e, int T_dim>
class comet_log_best: public comet_strategy<T_e, T_dim>{
protected:
    system<T_e, T_dim>* problem_;
    basic_scontainer<T_e, T_dim>* container_;
    std::string comet_api_key_;
    std::string workspace_;
    std::string project_;
    std::string experiment_name_;
    std::string experiment_link_;
    std::string experiment_key_;

public:
    comet_log_best(system<T_e, T_dim>* problem, basic_scontainer<T_e, T_dim>* container, std::string comet_api_key, std::string workspace, std::string project){
        this->problem_ = problem;
        this->container_ = container;
        this->comet_api_key_ = comet_api_key;
        this->workspace_ = workspace;
        this->project_ = project;
        this->create_experiment();
    }
    virtual void create_experiment(){
        spdlog::info("creating comet experiment...");
        // create the experiment
        nlohmann::json experiment_data = {{"workspaceName", this->workspace_},
                                          {"projectName", this->project_}};

        cpr::Response r = cpr::Post(cpr::Url{"https://www.comet.com/api/rest/v2/write/experiment/create"},
                                    cpr::Header{{"Authorization", this->comet_api_key_},
                                                {"Content-type", "application/json"},
                                                {"Accept", "application/json"}},
                                    cpr::Body{experiment_data.dump()});
        if(r.status_code != 200)
            spdlog::warn("comet request status code : {}", r.status_code);
        nlohmann::json rdata = nlohmann::json::parse(r.text);
        this->experiment_name_ = rdata["name"];
        this->experiment_link_ = rdata["link"];
        this->experiment_key_ = rdata["experimentKey"];
        spdlog::info("experiment name : {}", this->experiment_name_);
        spdlog::info("experiment link : {}", this->experiment_link_);
    }
    virtual void apply(){
         // find the best solution
        T_e best = *std::min_element(container_->values.begin(), container_->values.end());
        nlohmann::json metric_data = {{"experimentKey", this->experiment_key_},
                                      {"metricName", "objective"},
                                      {"metricValue", best}};

        auto resp = std::async([=](){
            cpr::Response r = cpr::Post(cpr::Url{"https://www.comet.com/api/rest/v2/write/experiment/metric"},
                                    cpr::Header{{"Authorization", this->comet_api_key_},
                                                {"Content-type", "application/json"},
                                                {"Accept", "application/json"}},
                                    cpr::Body{metric_data.dump()});
        });
    };
};

};
};
#endif