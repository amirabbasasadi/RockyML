#ifndef ROCKY_ZAGROS_GENETIC_STRATEGY
#define ROCKY_ZAGROS_GENETIC_STRATEGY
#include <rocky/zagros/strategies/strategy.h>
#include <set>
#include <iterator>

namespace rocky{
namespace zagros{

/**
 * @brief Base class for genetic mutations
 * 
 */
template<typename T_e, int T_dim>
class mutation_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief apply a mutation to k dimensions
 * 
 */
template<typename T_e, int T_dim>
class dimension_tweak_strategy: public mutation_strategy<T_e, T_dim>{
protected:
    // system
    system<T_e>* problem_;
    // main container
    basic_scontainer<T_e, T_dim>* target_container_;
    // candidate container
    basic_scontainer<T_e, T_dim>* candidates_;
    // number of mutations
    int n_mutations_;
    // number of affected dimensions
    int k_;
public:
    dimension_tweak_strategy(system<T_e>* problem, basic_scontainer<T_e, T_dim>* tgt_container, basic_scontainer<T_e, T_dim>* cnd_container, int k){
        this->problem_ = problem;
        this->target_container_ = tgt_container;
        this->candidates_ = cnd_container;
        this->k_ = k;
        this->n_mutations_ = cnd_container->n_particles();
    }
    virtual void tweak(int p_ind, int dim) = 0;
    virtual void apply(){
        tbb::parallel_for(0, n_mutations_, [this](int p){
            int samples[1];
            this->target_container_->sample_n_particles(samples, 1);
            std::copy(this->target_container_->particle(samples[0]),
                      this->target_container_->particle(samples[0])+T_dim,
                      this->candidates_->particle(p));
             // apply mutation on k dimensions
            for(int d=0; d<k_; d++){
                // choose a random dim
                int dim = this->target_container_->sample_dim();
                this->tweak(p, dim);
            }
        });
        target_container_->replace_with(candidates_);
    }
}; 


/**
 * @brief Gaussian mutation
 * 
 */
template<typename T_e, int T_dim>
class gaussian_mutation: public dimension_tweak_strategy<T_e, T_dim>{
protected:
    T_e mu_;
    T_e sigma_;
public:
    gaussian_mutation(system<T_e>* problem, basic_scontainer<T_e, T_dim>* tgt_container, basic_scontainer<T_e, T_dim>* cnd_container, int k=1, T_e mu=0.0, T_e sigma=0.5)
    :dimension_tweak_strategy<T_e, T_dim>(problem, tgt_container, cnd_container, k){
        this->mu_ = mu;
        this->sigma_ = sigma;
    }
    // generate gaussian noise
    T_e gaussian_noise(){
        static std::normal_distribution<T_e> dist(mu_, sigma_);
        auto z = dist(rocky::utils::random::prng());
        return z;
    }
    virtual void tweak(int p_ind, int dim){
        this->candidates_->particles[p_ind][dim] += gaussian_noise();
    }
};

/**
 * @brief Base class for genetic crossovers
 * 
 */
template<typename T_e, int T_dim>
class crossover_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief Multipoint crossover
 * 
 */
template<typename T_e, int T_dim>
class multipoint_crossover: public crossover_strategy<T_e, T_dim>{
protected:
    // system
    system<T_e>* problem_;
    // main container
    basic_scontainer<T_e, T_dim>* container_;
    // condidates container
    basic_scontainer<T_e, T_dim>* candidates_;
    // number of crossovers
    int n_crossovers_;
    // maximum number of affected dimensions
    int k_;
public:
    multipoint_crossover(system<T_e>* problem, basic_scontainer<T_e, T_dim>* container, basic_scontainer<T_e, T_dim>* candidates, int k){
        this->k_ = k;
        this->problem_ = problem;
        this->container_ = container;
        this->candidates_ = candidates;
        this->n_crossovers_ = candidates->n_particles() / 2;
    }
    virtual void apply(){
        tbb::parallel_for(0, n_crossovers_, [this](int p){
            int parents[2];
            this->container_->sample_n_particles(parents, 2);
            // copy thee parents
            for(int i=0; i<2; i++)
                std::copy(this->container_->particle(parents[i]),
                          this->container_->particle(parents[i])+T_dim,
                          this->candidates_->particle(2*p+i));
            // affected dims
            std::set<int> dims;
            for(int d=0; d<k_; d++)
                dims.insert(this->container_->sample_dim());
            // apply the crossover
            for(auto dim: dims)
                std::swap(this->candidates_->particles[parents[0]][dim],
                          this->candidates_->particles[parents[1]][dim]);
        });
        container_->replace_with(candidates_);
    }
};


template<typename T_e, int T_dim>
class static_segment_crossover: public crossover_strategy<T_e, T_dim>{
protected:
    // system
    system<T_e>* problem_;
    // main container
    basic_scontainer<T_e, T_dim>* container_;
    // main container
    basic_scontainer<T_e, T_dim>* candidates_;
    // length of the segment for crossover
    int segment_length_;
    // number of crossovers
    int n_crossovers_;
public:
    static_segment_crossover(system<T_e>* problem, basic_scontainer<T_e, T_dim>* container, basic_scontainer<T_e, T_dim>* cnd_container, int n_crossovers, int segment_length){
        this->n_crossovers_ = n_crossovers;
        this->segment_length_ = segment_length;
        this->problem_ = problem;
        this->container_ = container;
        this->candidates_ = cnd_container;
    }
    virtual void apply(){
        tbb::parallel_for(0, n_crossovers_, [this](auto ci){
            // select two distinct parents
            int parents[2];
            this->container_->sample_n_particles(parents, 2);
            // select a random point for cross over
            std::uniform_int_distribution<> point_dist(0, T_dim - segment_length_ - 1);
            int point = point_dist(rocky::utils::random::prng());
            // produce two cantidates
            for(int i=0; i<2; i++){
                // copy the solution
                std::copy(this->container_->particle(parents[i]),
                          this->container_->particle(parents[i])+T_dim,
                          this->candidates_->particle(2*ci+i));
                // replace the segment
                std::copy(this->container_->particle(parents[1-i])+point,
                          this->container_->particle(parents[1-i])+point+segment_length_,
                          this->candidates_->particle(2*ci+i)+point); 
            }
        });
        // evaluate the candidates
        this->candidates_->evaluate_and_update(problem_);
        this->container_->replace_with(this->candidates_);
    }
};

};
};
#endif