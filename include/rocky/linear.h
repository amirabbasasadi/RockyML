#ifndef ROCKY_LINEAR_GUARD
#define ROCKY_LINEAR_GUARD
#include <Fastor/Fastor.h>
#include <type_traits>
#include <algorithm>
namespace rocky{
namespace layer{
enum opt {bias, no_bias};

/**
 * @brief base class for static layers
 * 
 */
template<typename T_e, size_t T_in_dim, size_t T_in_num, size_t T_out_dim,
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
};
};
#endif