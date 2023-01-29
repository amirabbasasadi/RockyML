/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_ZAGROS_EDA_STRATEGY
#define ROCKY_ZAGROS_EDA_STRATEGY


#include <rocky/zagros/strategies/strategy.h>

#include <Eigen/Core>
#include <Eigen/Cholesky>


namespace rocky{
namespace zagros{

/**
 * @brief Base class for estimation of distribution algorithms
 * 
 */
template<typename T_e, int T_dim>
class eda_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief estimating the distribution of solutions using eda
 * 
 */
template<typename T_e, int T_dim>
class eda_mutivariate_normal: public eda_strategy<T_e, T_dim>{
protected:
    // system
    system<T_e>* problem_;
    // main container
    basic_scontainer<T_e, T_dim>* target_container_;
    // container holding the generated candidates
    basic_scontainer<T_e, T_dim>* candidates_container_;
    // number of generated candidates
    std::vector<T_e> top_particles_mem_; 
    // a placeholder for top-k solutions
    std::vector<int> solution_ind_;
    // covariance matrix
    std::vector<T_e> cov_mem_;
    // mean vector
    std::vector<T_e> mean_mem_;
    // sample size for computing covariance matrix
    int sample_size_;
    // number of generated candidates in each step
    int n_candidates_;

public:
    eda_mutivariate_normal(system<T_e>* problem, basic_scontainer<T_e, T_dim>* tgt_container, basic_scontainer<T_e, T_dim>* cnd_container, int sample_size){
        this->problem_ = problem;
        this->target_container_ = tgt_container;
        this->candidates_container_ = cnd_container;
        this->sample_size_ = sample_size;
        this->n_candidates_ = cnd_container->n_particles();
        top_particles_mem_.resize(sample_size * T_dim);
        solution_ind_.resize(sample_size);
        cov_mem_.resize(T_dim * T_dim);
        mean_mem_.resize(T_dim);
    }
    void sample_std_normal(T_e* vec){
        static std::normal_distribution<T_e> dist(0.0, 1.0);
        tbb::parallel_for(0, T_dim, [&](auto i){
            vec[i] = dist(rocky::utils::random::prng());
        });
    }
    virtual void apply(){
        // find top k solutions
        target_container_->best_k(solution_ind_.data(), sample_size_);
        // copy the best soluions
        T_e* top_particles_ptr = top_particles_mem_.data();

        tbb::parallel_for(0, sample_size_, [&](int p){
            std::copy(this->target_container_->particle(this->solution_ind_[p]),
                      this->target_container_->particle(this->solution_ind_[p]) + T_dim,
                      top_particles_ptr + p * T_dim);
        });
        // estimate the mean and covariance
        Eigen::Map<Eigen::Matrix<T_e, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> top_particles_mat(top_particles_mem_.data(), sample_size_, T_dim);
        Eigen::Map<Eigen::Matrix<T_e, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> cov_mat(cov_mem_.data(), T_dim, T_dim);
        Eigen::Map<Eigen::Matrix<T_e, 1, T_dim, Eigen::RowMajor>> mean_mat(mean_mem_.data());

        mean_mat = top_particles_mat.colwise().mean();
        top_particles_mat.rowwise() -= mean_mat;
        cov_mat = (top_particles_mat.adjoint() * top_particles_mat) / static_cast<T_e>(sample_size_-1);
        // cholesky decomposition
        Eigen::LLT<Eigen::Matrix<T_e, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> llt(cov_mat);
        // generate samples from mvn
        tbb::parallel_for(0, sample_size_, [&](auto p){
            Eigen::Map<Eigen::Matrix<T_e, 1, T_dim, Eigen::RowMajor>> sample_mat(this->candidates_container_->particle(p));
            sample_std_normal(this->candidates_container_->particle(p));
            sample_mat = (llt.matrixL() * sample_mat.transpose()).transpose();
            sample_mat.rowwise() += mean_mat;
        }); 
        // evaluate generated candidates
        this->candidates_container_->evaluate_and_update(this->problem_);
        // replace the best candidates in the target container
        this->target_container_->replace_with(this->candidates_container_);
   }
}; 



}; // end of zagros
}; // end of rocky
#endif