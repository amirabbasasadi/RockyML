#ifndef ROCKY_BLOCK_GUARD
#define ROCKY_BLOCK_GUARD
#include <Fastor/Fastor.h>
#include <type_traits>
#include <algorithm>
namespace rocky{
namespace blocks{
enum opt {bias, no_bias};

/**
 * @brief base class for static layers
 * 
 */
template<typename T_e, size_t T_in_num, size_t T_in_dim, size_t T_out_dim,
        opt T_opt_bias=opt::bias>
class linear{
public:
    constexpr size_t deduce_num_params_weights(){
        return T_in_dim * T_out_dim;
    }
    constexpr size_t deduce_num_params_bias(){
        if constexpr(T_opt_bias == opt::bias)
            return T_out_dim;
        else
            return 0;
    }
    constexpr size_t deduce_num_params(){
        return deduce_num_params_weights() + deduce_num_params_bias(); 
    }
    /**
     * (T_in_num * T_in_dim) @ (T_in_dim x T_out_dim) -> (T_in_num * T_out_dim)
     * **/ 
    void feed(T_e* layer_mem_ptr, T_e* in_mem_ptr, T_e* out_mem_ptr){
        Fastor::TensorMap<T_e, T_in_dim, T_out_dim> W_(layer_mem_ptr);
        Fastor::TensorMap<T_e, T_in_num, T_in_dim> In_(in_mem_ptr);
        Fastor::TensorMap<T_e, T_in_num, T_out_dim> Out_(out_mem_ptr);
        Out_ = Fastor::matmul(In_, W_);
        if constexpr (T_opt_bias == opt::bias){
            for(int i=0; i<T_in_num; i++)
                for(int j=0; j<T_out_dim; j++)
                    out_mem_ptr[i*T_out_dim + j] += layer_mem_ptr[T_in_dim * T_out_dim + j];
        }   
    }
}; // end linear

template<typename T_e, size_t T_layers_num,
         size_t T_in_num, size_t T_in_dim,
         size_t T_out_dim, size_t T_hidden_dim,
         opt T_opt_bias=opt::bias>
class mlp{
public:
    constexpr size_t deduce_num_params_in(){
        return linear<T_e, T_in_num, T_in_dim, T_hidden_dim, T_opt_bias>().deduce_num_params();
    }
    constexpr size_t deduce_num_params_hidden(){
        return linear<T_e, T_in_num, T_hidden_dim, T_hidden_dim, T_opt_bias>().deduce_num_params();
    }
    constexpr size_t deduce_num_params_out(){
        return linear<T_e, T_in_num, T_hidden_dim, T_out_dim, T_opt_bias>().deduce_num_params();
    }
    constexpr size_t deduce_num_params(){
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
        size_t offset = l_in.deduce_num_params();
        for (size_t hidden=0; hidden<T_layers_num; hidden++){
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