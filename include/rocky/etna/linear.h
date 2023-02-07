#ifndef ROCKY_ETNA_LINEAR
#define ROCKY_ETNA_LINEAR

#include <Eigen/Core>
#include <type_traits>
#include <algorithm>

namespace rocky{
namespace etna{
enum opt {bias, no_bias};

/**
 * @brief base class for static layers
 * 
 */
template<typename T_e, int T_in_num, int T_in_dim, int T_out_dim,
        opt T_opt_bias=opt::bias>
class linear{
public:
    static constexpr int deduce_num_params_weights(){
        return T_in_dim * T_out_dim;
    }
    static constexpr int deduce_num_params_bias(){
        if constexpr(T_opt_bias == opt::bias)
            return T_out_dim;
        else
            return 0;
    }
    static constexpr int deduce_num_params(){
        return deduce_num_params_weights() + deduce_num_params_bias(); 
    }
    /**
     * (T_in_num * T_in_dim) @ (T_in_dim x T_out_dim) -> (T_in_num * T_out_dim)
     * **/ 
    void feed(T_e* layer_mem_ptr, T_e* in_mem_ptr, T_e* out_mem_ptr){
        Eigen::Map<Eigen::Matrix<T_e, T_in_dim, T_out_dim, Eigen::RowMajor>> W_(layer_mem_ptr);
        Eigen::Map<Eigen::Matrix<T_e, T_in_num, T_in_dim, Eigen::RowMajor>> In_(in_mem_ptr);
        Eigen::Map<Eigen::Matrix<T_e, T_in_num, T_out_dim, Eigen::RowMajor>> Out_(out_mem_ptr);
        Out_ = In_ * W_;
        // adding bias to each row
        if constexpr (T_opt_bias == opt::bias){
            Eigen::Map<Eigen::Matrix<T_e, 1, T_out_dim, Eigen::RowMajor>> Bias_(layer_mem_ptr + T_in_dim * T_out_dim);
            Out_.rowwise() += Bias_; 
        }   
    }
}; // end linear

template<typename T_e, int T_layers_num,
         int T_in_num, int T_in_dim,
         int T_out_dim, int T_hidden_dim,
         opt T_opt_bias=opt::bias>
class mlp{
public:
    static constexpr int deduce_num_params_in(){
        return linear<T_e, T_in_num, T_in_dim, T_hidden_dim, T_opt_bias>::deduce_num_params();
    }
    static constexpr int deduce_num_params_hidden(){
        return linear<T_e, T_in_num, T_hidden_dim, T_hidden_dim, T_opt_bias>::deduce_num_params();
    }
    static constexpr int deduce_num_params_out(){
        return linear<T_e, T_in_num, T_hidden_dim, T_out_dim, T_opt_bias>::deduce_num_params();
    }
    static constexpr int deduce_num_params(){
        return T_layers_num * deduce_num_params_hidden() + deduce_num_params_in() + deduce_num_params_out();
    }
    /**
     * @brief apply the multi-layer perceptron on data in `in_mem_ptr`
     * 
     * @param layer_mem_ptr memory block containing layer parameters
     * @param in_mem_ptr  memory block containing input data
     * @param out_mem_ptr memory block for storing the result
     * @return ** void 
     * 
     */
    void feed(T_e* layer_mem_ptr, T_e* in_mem_ptr, T_e* out_mem_ptr){
        // layers
        linear<T_e, T_in_num, T_in_dim, T_hidden_dim, T_opt_bias> l_in;
        linear<T_e, T_in_num, T_hidden_dim, T_hidden_dim, T_opt_bias> l_hidden;
        linear<T_e, T_in_num, T_hidden_dim, T_out_dim, T_opt_bias> l_out;
        // reserving space for intermediate matrices
        T_e* H1_ = new T_e[T_in_num * T_hidden_dim];
        T_e* H2_ = new T_e[T_in_num * T_hidden_dim];
        // apply input layer
        l_in.feed(layer_mem_ptr, in_mem_ptr, H1_);
        // apply hidden layers
        T_e* src, *dest;
        int offset = l_in.deduce_num_params();
        for (int hidden=0; hidden<T_layers_num; hidden++){
            if (hidden % 2 == 0){ src = H1_; dest = H2_;}
            else{ src = H2_; dest = H1_;}
            l_hidden.feed(layer_mem_ptr + offset, src, dest);
            offset += l_hidden.deduce_num_params();
        }    
        // apply output layer
        if constexpr (T_layers_num % 2 == 0)
            l_out.feed(layer_mem_ptr + offset, H1_, out_mem_ptr);
        else
            l_out.feed(layer_mem_ptr + offset, H2_, out_mem_ptr);
        
        delete[] H1_;
        delete[] H2_;
    }

};

};
};

#endif