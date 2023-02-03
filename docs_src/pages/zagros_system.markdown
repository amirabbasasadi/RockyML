# Define a custom optimization problem in Zagros{#zagros_system}
To define your optimization problem, you have to define a Zagros *system*. Here is an example of defining a system:
```cpp
template<typename T_e>
class my_system: public zagros::system<T_e>{
public:
    virtual T_e objective(T_e* solution){
        // this method must be implemented
    }
};
```
**Important Note**: Zagros systems MUST be thread-safe. I warned you! 
Since Zagros search strategies are gradient-free, you can do anything inside `objective`. For example you can use Eigen for matrix computation:
```cpp
using namespace Eigen;

template<typename T_e>
class my_system: public zagros::system<T_e>{
protected:
    int m;
    int n;
public:
    virtual T_e objective(T_e* solution){
        // create an mxn matrix
        Map<Matrix<T_e, Dynamic, Dynamic>> mat(solution, m, n);
        // do something with the matrix
    }
};
```
In general it's a good performance practice to avoid making copies. for example in the above code, Eigen does not create a copy and works directly on the `solution`.
You can also provide lower-bound and upper-bound for your problem:
```cpp
template<typename T_e>
class my_system: public zagros::system<T_e>{
public:
    virtual T_e objective(T_e* solution){
        // this method must be implemented
    }
    virtual T_e lower_bound(){
        return -5.0;
    }
    virtual T_e upper_bound(){
        return 5.0;
    }
};
```
If you need, you can specify lower bound and upper bound for each parameter:
```cpp
template<typename T_e>
class my_system: public zagros::system<T_e>{
public:
    virtual T_e objective(T_e* solution){
        // this method must be implemented
    }
    virtual T_e lower_bound(int param_index){
        if(param_index == 2)
            return -4.0;
        return -5.0;
    }
    virtual T_e upper_bound(int param_index){
        return 5.0;
    }
};
```