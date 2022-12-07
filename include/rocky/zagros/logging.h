#ifndef ROCKY_ZAGROS_LOG_GUARD
#define ROCKY_ZAGROS_LOG_GUARD
#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>

namespace rocky{
namespace zagros{

/**
 * Logging interface for tracking a certian quantity 
 * 
 */
class optimization_log{
protected:
    std::fstream log_output_;
    std::map<std::string, std::string> meta_data_;
public:
    /**
     * @brief will be called just after openning the file
     * should be used to add header or other meta-data at the beginning of the log file
     * 
     * @return ** void 
     */
    virtual void log_on_open(){}
    /**
     * @brief will be called after each optimization step 
     * supposed to track the result of optimization
     * 
     * @return ** void 
     */
    virtual void log_step(int time) = 0;
    /**
     * @brief will be called before closing the file
     * 
     * @return ** void 
     */
    virtual void log_on_close(){}
     /**
     * @brief initialize logging
     * openning the log file and adding headers and etc.
     * 
     * @param path a string containing the path of log output
     * @return ** void 
     */
    virtual void log_init(std::string path){
        // openning the output file
        log_output_.open(path, std::ofstream::out);
        log_on_open();
    }
    /**
     * @brief add metadata
     * 
     */
     template<typename T_val_type>
     void log_metadata(std::string key, T_val_type value){
        meta_data_[key] = std::to_string(value);  
     }
    /**
     * @brief close the output file and save the result
     * 
     */
    virtual void log_save(){
        // add metadata
        log_output_ << "---" << std::endl;
        for(auto const& [key, value] : meta_data_)
            log_output_ << key << ":" << value << std::endl;
        log_on_close();
        log_output_.close();
    }
};

};
};

#endif