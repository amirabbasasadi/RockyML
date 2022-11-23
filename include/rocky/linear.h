#ifndef ROCKY_LINEAR_GUARD
#define ROCKY_LINEAR_GUARD
#include <cblas.h>
#include <type_traits>
#include <algorithm>
namespace rocky{
namespace layer{

enum opt {bias, no_bias};
template<typename T_e, unsigned T_in, unsigned T_out,
        opt T_bias_option=opt::bias>
class linear{
protected:
    constexpr unsigned deduce_num_parameters(){
        if constexpr (T_bias_option == opt::bias)
            return (T_in+1) * T_out;
        return (T_in) * T_out;
    }
    const unsigned num_parameters_ = deduce_num_parameters();
public:
    constexpr unsigned num_parameters() const{ return num_parameters_; }
    /**
     * (B_in * T_in) @ (T_in x T_out) -> (B_in * T_out)
     * **/ 
    void apply(T_e* layer_mem_ptr, T_e* in_mem_ptr, unsigned B_in, T_e* out_mem_ptr){
        // filling output matrix with bias vectors
        T_e beta = 0.0;
        if constexpr (T_bias_option == opt::bias){
            T_e* bias = layer_mem_ptr + T_in*T_out;
            for (int r=0; r<B_in; r++)
                std::copy(bias, bias+T_out, out_mem_ptr + r*T_out); 
            beta = 1.0;
        }
        // single precision matrix multiplication
        if constexpr (std::is_same<T_e, float>::value)
            cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                B_in, T_out, T_in, 1.0, in_mem_ptr, T_in, layer_mem_ptr, T_out,
                beta, out_mem_ptr, T_out);
        // double precision version
        if constexpr (std::is_same<T_e, double>::value)
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                B_in, T_out, T_in, 1.0, in_mem_ptr, T_in, layer_mem_ptr, T_out,
                beta, out_mem_ptr, T_out);

    }
}; // end linear

};
};

#endif