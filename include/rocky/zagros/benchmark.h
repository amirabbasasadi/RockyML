/*
    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
    distributed under Apache-2.0 license
*/
#ifndef ROCKY_BENCHMARK_GUARD
#define ROCKY_BENCHMARK_GUARD
#define _USE_MATH_DEFINES

#include<string>
#include<sstream>
#include<cmath>
#include<random>
#include<algorithm>
#include<numeric>
#include<fstream>

#include<tbb/tbb.h>
#include<Eigen/Core>
#include<Eigen/QR>


#include<rocky/zagros/system.h>
#include<rocky/utils.h>

namespace rocky{
namespace zagros
{
namespace benchmark
{
/**
 * @brief Sphere function
 * 
 */
template<typename T_e>
class sphere: public rocky::zagros::system<T_e>{
protected:
    int dim_;
public:
    sphere(int dim){
        dim_ = dim;
    }
    virtual T_e objective(T_e* x){
        T_e S = 0.0;
        for(size_t i=0; i<dim_; i++)
            S += x[i] * x[i];
        return sqrt(S);
    }
    virtual T_e lower_bound(){ return -10.0; }
    virtual T_e upper_bound(){ return 10.0; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "Sphere(dim=" << dim_ << ")";
        return name.str();
    }
};

/**
 * @brief Rosenbrock function
 * 
 */
template<typename T_e>
class rosenbrock: public rocky::zagros::system<T_e>{
protected:
    int dim_;

public:
    rosenbrock(int dim=2){
        dim_ = dim;
    }
    virtual T_e objective(T_e* x){
        T_e S = 0;
        for(size_t i=0; i<dim_-1; i++)
            S += 100.0 * pow(x[i+1] - pow(x[i], 2), 2) + pow(1 - x[i], 2);
        return S;
    }
    virtual T_e lower_bound(){ return -10.0; }
    virtual T_e upper_bound(){ return 10.0; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "Rosenbrock(dim=" << dim_ << ")";
        return name.str();
    }
};

template<typename T_e>
class rastrigin: public rocky::zagros::system<T_e>{
protected:
    int dim_;
    T_e shift_;
public:
    /**
     * @brief Construct a new rastrigin object
     * 
     * @param dim dimension of rastrigin function
     * @param shift the shift applied to global minimum
     */
    rastrigin(int dim=2, T_e shift=0.0){
        dim_ = dim;
        shift_ = shift;
    }
    virtual T_e objective(T_e* x){
        T_e S = 10.0 * dim_;
        for(size_t i=0; i<dim_; i++)
            S += (x[i]-shift_) * (x[i]-shift_) - 10.0*cos(2*M_PI * (x[i]-shift_));
        return S;
    }
    virtual T_e lower_bound(){ return -5.12; }
    virtual T_e upper_bound(){ return 5.12; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "Rastrigin(dim=" << dim_ << ")";
        return name.str();
    }
};

template<typename T_e>
class rastrigin_parallel: public rocky::zagros::system<T_e>{
protected:
    int dim_;
    T_e shift_;
public:
    /**
     * @brief Construct a new rastrigin object
     * 
     * @param dim dimension of rastrigin function
     * @param shift the shift applied to global minimum
     */
    rastrigin_parallel(int dim=2, T_e shift=0.0){
        dim_ = dim;
        shift_ = shift;
    }
    virtual T_e objective(T_e* x){
        T_e S = 10.0 * dim_;
        tbb::combinable<T_e> partial_sums{0.0};
        tbb::parallel_for(0, dim_, [&](auto i){
            partial_sums.local() += (x[i]-shift_) * (x[i]-shift_) - 10.0*cos(2*M_PI * (x[i]-shift_));
        });       
        S += partial_sums.combine([](auto x, auto y){ return x+y; });
        return S;
    }
    virtual T_e lower_bound(){ return -5.12; }
    virtual T_e upper_bound(){ return 5.12; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "Parallel Rastrigin(dim=" << dim_ << ")";
        return name.str();
    }
};

template<typename T_e>
class ackley: public rocky::zagros::system<T_e>{
protected:
    int dim_;

public:
    ackley(int dim=2){
        dim_ = dim;
    }
    virtual T_e objective(T_e* x){
        T_e S = 20.0 + exp(1.0);
        T_e S_c = 0;
        T_e S_s = 0;
        for(size_t i=0; i<dim_; i++){
            S_s += pow(x[i], 2);
            S_c += cos(2*M_PI * x[i]);
        }
        S_s /= dim_;
        S_c /= dim_;
        S_s = -0.20 * sqrt(S_s);
        S_s = -20.0 * exp(S_s);
        S_c = -exp(S_c);
        
        S += S_s + S_c;
        return S;
    }
    virtual T_e lower_bound(){ return -5.0; }
    virtual T_e upper_bound(){ return 5.0; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "Ackley(dim=" << dim_ << ")";
        return name.str();
    }
};

/**
 * @brief Griewank function
 * reference: https://www.sfu.ca/~ssurjano/griewank.html
 */
template<typename T_e>
class griewank: public rocky::zagros::system<T_e>{
protected:
    int dim_;
    T_e lb_;
    T_e ub_;

public:
    /**
     * @brief Construct a new griewank object
     * 
     * @param dim dimension
     * @param lb lower bound
     * @param ub upper bound
     */
    griewank(int dim=2, T_e lb=-20.0, T_e ub=20.0){
        dim_ = dim;
        lb_ = lb;
        ub_ = ub;
    }
    virtual T_e objective(T_e* x){
        T_e S = 1;
        T_e S_p = 0;
        T_e P_c = 1.0;
        for(size_t i=0; i<dim_; i++){
            S_p += pow(x[i], 2)/4000.0;
            P_c *= cos(x[i]/sqrt(i+1));
        }
        S += S_p - P_c;
        return S;
    }
    virtual T_e lower_bound(){ return lb_; }
    virtual T_e upper_bound(){ return ub_; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "griewank(dim=" << dim_ << ")";
        return name.str();
    }
};

/**
 * @brief Dropwave function
* referenc : https://www.sfu.ca/~ssurjano/drop.html
 * 
 */
template<typename T_e>
class dropwave: public rocky::zagros::system<T_e>{
protected:
    int dim_;
    T_e lb_;
    T_e ub_;

public:
    /**
     * @brief Construct a new dropwave problem
     * 
     * @param dim dimension
     */
    dropwave(int dim=2){
        dim_ = dim;
    }
    virtual T_e objective(T_e* x){
        T_e S_s = 0;
        for(size_t i=0; i<dim_; i++)
            S_s += pow(x[i], 2);
        T_e value = -(1 + cos(12.0 * sqrt(S_s)))/(0.5 * S_s + 1);
        return value;
    }
    virtual T_e lower_bound(){ return -5.12; }
    virtual T_e upper_bound(){ return 5.12; }
    virtual std::string to_string(){
        std::stringstream name;
        name << "dropwave(dim=" << dim_ << ")";
        return name.str();
    }
};



template<typename T_e>
struct thread_safe_least_squares{
public:
    std::vector<T_e> A_;
    std::vector<T_e> b_;

    thread_safe_least_squares(int m,  int n){
        std::mt19937 local_rng(0);        
        A_.resize(m * n);
        b_.resize(m);
        std::uniform_real_distribution<T_e> dist(-10.0, 10.0);
        for(int i=0; i<m; i++)
            b_[i] = dist(local_rng);

        for(int i=0; i<m*n; i++)
            A_[i] = dist(local_rng);
    }
};

template<typename T_e>
class least_squares: public rocky::zagros::system<T_e>{
protected:
    int m_;
    int n_;
    tbb::enumerable_thread_specific<thread_safe_least_squares<T_e>> problem_;

public:
    least_squares(int m, int n): problem_(m, n){
        m_ = m;
        n_ = n;
    }
    virtual T_e objective(T_e* x_){
        T_e* A_ = problem_.local().A_.data();
        T_e* b_ = problem_.local().b_.data();
     
        Eigen::Map<Eigen::Matrix<T_e, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> A(A_, m_, n_);
        Eigen::Map<Eigen::Matrix<T_e, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> b(b_, m_, 1);
        Eigen::Map<Eigen::Matrix<T_e, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> x(x_, n_, 1);
        
        auto error = (A*x - b).norm();

        return error;
    }
    virtual T_e lower_bound(){ return -20.0; }
    virtual T_e upper_bound(){ return 20.0; }
};

}; // namespace benchmark
        
}; // namespace zagros
    
}; // namespace rocky

#endif