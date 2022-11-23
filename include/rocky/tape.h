#ifndef ROCKY_TAPE_GUARD
#define ROCKY_TAPE_GUARD
#include<type_traits>
#include<algorithm>

namespace rocky{
/**
 * @brief base class for tapes
 * 
 */
template<typename T_e>
class basic_tape{};

/**
 * @brief dynamic data tape
 * 
 */
template<typename T_e>
class dynamic_tape: basic_tape<T_e>{
private:
    T_e* mem_block_;
    size_t size_;
public:
    dynamic_tape(size_t size){
        size_ = size;
        mem_block_ = new T_e[size];
        if(!mem_block_){
            // throw exception
        }
    }
    size_t size() const{ return size_; }
};

}; // end rocky

#endif