#ifndef ROCKY_ZAGROS_DE_STRATEGY
#define ROCKY_ZAGROS_DE_STRATEGY

#include <map>
#include <set>

#include <rocky/zagros/strategies/strategy.h>

namespace rocky{
namespace zagros{

template<typename T_e, int T_dim>
class basic_differnetial_evolution: public search_strategy<T_e, T_dim>{
protected:
    system<T_e, T_dim>* problem_;
    basic_scontainer<T_e, T_dim>* container_;
    // DE hyper-parameters
    T_e CR_;
    T_e DW_;

public:
    typedef Eigen::Map<Eigen::Matrix<T_e, 1, T_dim, Eigen::RowMajor>> eigen_particle;

    basic_differnetial_evolution(system<T_e, T_dim>* problem, basic_scontainer<T_e, T_dim>* container){
        this->problem_ = problem;
        this->container_ = container;
        this->CR_ = 0.5;
        this->DW_ = 1.0;
    }
    /**
     * @brief generate a uniform random number
     * 
     * @return * T_e 
     */
    T_e rand_uniform(){
        static std::uniform_real_distribution<T_e> dist(0.0, 1.0);
        return dist(rocky::utils::random::prng);
    }    
    // apply differential evolution within groups in parallel
    virtual void apply(){
        tbb::parallel_for(0, this->container_->n_groups(), [this](int group){
            std::map<int, T_e> backup;
            std::set<int> indices = this->container_->sample_n_particles(4, group);
            auto ind_it = indices.begin();
            int x_ind = *ind_it;
            int a_ind = *std::next(ind_it, 1);
            int b_ind = *std::next(ind_it, 2);
            int c_ind = *std::next(ind_it, 3);
            eigen_particle x(this->container_->particle(x_ind));
            eigen_particle a(this->container_->particle(a_ind));
            eigen_particle b(this->container_->particle(b_ind));
            eigen_particle c(this->container_->particle(c_ind));
            for(int d=0; d<T_dim; d++){
                // perform crossover based on the crossover probability CR
                if(this->rand_uniform() > this->CR_)
                    continue;
                // apply crossover
                backup[d] = x(0, d);
                x(0, d) = a(0, d) * this->DW_ * (b(0, d) - c(0, d));            
            }
            // keep the new solution if there was any improvement
            T_e new_val = this->problem_->objective(this->container_->particle(x_ind));
            // restore the previous particle otherwise 
            if (new_val > this->container_->values[x_ind])
                for(auto const& [dim, val]: backup)
                    x(0,dim) = val;
            else  // replace the min value
                this->container_->values[x_ind] = new_val;
        });
    }
};


};
};

#endif