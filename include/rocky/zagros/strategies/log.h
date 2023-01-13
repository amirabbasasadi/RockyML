#ifndef ROCKY_ZAGROS_LOG_STRATEGY
#define ROCKY_ZAGROS_LOG_STRATEGY

#ifdef ROCKY_USE_MPI
#include<mpi.h>
#include<stdio.h>
#endif

#include<rocky/zagros/strategies/strategy.h>
#include<rocky/zagros/strategies/communication.h>
#include<nlohmann/json.hpp>
#include<cpr/cpr.h>

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
 * @brief Log the best solution in the container in a csv file
 * 
 */
template<typename T_e, int T_dim>
class local_log_best: public logging_strategy<T_e, T_dim>{
protected:
    system<T_e>* problem_;
    basic_scontainer<T_e, T_dim>* container_;
    std::fstream log_output_;
    bool log_groups_;
    size_t step_;

public:
    local_log_best(system<T_e>* problem, basic_scontainer<T_e, T_dim>* container, std::string filename, bool log_groups=false){
        this->problem_ = problem;
        this->container_ = container;
        this->step_ = 0;
        this->log_groups_ = log_groups;
        // create a specific filename for this rank
        int mpi_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
        std::string process_spc_filename = fmt::format("proc_{}_{}", mpi_rank, filename);
        // initialize the output file
        this->log_output_.open(process_spc_filename, std::fstream::out);
        // write the headers
        this->write_header();
    }
    virtual void write_header(){
        if (this->log_groups_)
            this->log_output_ << "step,group,best" << std::endl;
        else
            this->log_output_ << "step,best" << std::endl;
    }
    virtual void apply(){
        // find the best solution
        T_e best = container_->best_min();
        this->log_output_ << fmt::format("{},{}", this->step_, best) << std::endl;
        this->step_++;
    };
    virtual void save(){
        if(this->log_output_.is_open())
            this->log_output_.close();
    }
     virtual ~local_log_best(){
        this->save();
    }
};

/**
 * @brief Interface for Comet logging strategies
 * 
 */
template<typename T_e, int T_dim>
class comet_strategy: public logging_strategy<T_e, T_dim>{};

/**
 * @brief A strategy for logging the best solution on Comet server
 * 
 */
template<typename T_e, int T_dim>
class comet_log_best: public comet_strategy<T_e, T_dim>{
protected:
    system<T_e>* problem_;
    basic_scontainer<T_e, T_dim>* container_;
    std::string comet_api_key_;
    std::string workspace_;
    std::string project_;
    std::string metric_name_;
    std::string experiment_name_;
    std::string experiment_link_;
    std::string experiment_key_;

public:
    comet_log_best(system<T_e>* problem, basic_scontainer<T_e, T_dim>* container, std::string comet_api_key, std::string workspace, std::string project, std::string metric_name){
        this->problem_ = problem;
        this->container_ = container;
        this->comet_api_key_ = comet_api_key;
        this->workspace_ = workspace;
        this->project_ = project;
        this->metric_name_ = metric_name;
         // we need to make sure in an MPI app only the root process will create the experiment        
        this->create_experiment();        
    }
    void connection_warning(int status_code){
        if(status_code != 200)
            spdlog::warn("Error while connecting Comet server! request status code : {}", status_code);
    }
    void create_experiment(){
        spdlog::info("creating comet experiment...");
        // encoding experiment data
        nlohmann::json experiment_data = {{"workspaceName", this->workspace_},
                                          {"projectName", this->project_}};
        // sending a post request for creating the experiment
        cpr::Response r = cpr::Post(cpr::Url{"https://www.comet.com/api/rest/v2/write/experiment/create"},
                                    cpr::Header{{"Authorization", this->comet_api_key_},
                                                {"Content-type", "application/json"},
                                                {"Accept", "application/json"}},
                                    cpr::Body{experiment_data.dump()});
        // warn the user if connection wasn't successful
        this->connection_warning(r.status_code);
        nlohmann::json rdata = nlohmann::json::parse(r.text);
        // store the exepriment information
        this->experiment_name_ = rdata["name"];
        this->experiment_link_ = rdata["link"];
        // experiment key will be needed for submitting metric values
        this->experiment_key_ = rdata["experimentKey"];
        std::string proc_metric_name = get_metric_name();
        spdlog::info("{} experiment name : {}", proc_metric_name, this->experiment_name_);
        spdlog::info("{} experiment link : {}", proc_metric_name, this->experiment_link_);
        spdlog::info("{} experiment key : {}", proc_metric_name, this->experiment_key_);
    }
    #ifdef ROCKY_USE_MPI
    void broadcast_experiment(){
        // root process should broadcast the experiment info
        int mpi_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
        // a buffer for experiment key
        char key_buffer[33];
        if(mpi_rank == 0)
            sprintf(key_buffer, "%.32s", experiment_key_.c_str());
        spdlog::info("broadcasting experiment information to all processes...");
        MPI_Bcast(key_buffer, 32, MPI_CHAR, 0, MPI_COMM_WORLD);
        if(mpi_rank != 0){
            experiment_key_ = std::string(key_buffer);
            spdlog::info("process {} has the key : {}", mpi_rank, experiment_key_);
        }
    }
    #endif
    std::string get_metric_name(){
        std::string name = metric_name_;
        #ifdef ROCKY_USE_MPI
            int mpi_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
            name = std::string("proc_") + std::to_string(mpi_rank) + "_" + name;
        #endif
        return name;
    }
    virtual void apply(){
         // find the best solution
        T_e best = container_->best_min();
        // encode the metric data
        nlohmann::json metric_data = {{"experimentKey", this->experiment_key_},
                                      {"metricName", this->get_metric_name()},
                                      {"metricValue", best}};
        // send metric data using an async call
        auto req_future = std::async([=](){
            cpr::Response r = cpr::Post(cpr::Url{"https://www.comet.com/api/rest/v2/write/experiment/metric"},
                                cpr::Header{{"Authorization", this->comet_api_key_},
                                            {"Content-type", "application/json"},
                                            {"Accept", "application/json"}},
                                cpr::Body{metric_data.dump()});
            this->connection_warning(r.status_code);
        });
        
    };
};

};
};
#endif