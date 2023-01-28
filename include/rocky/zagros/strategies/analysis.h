#ifndef ROCKY_ZAGROS_ANALYSIS
#define ROCKY_ZAGROS_ANALYSIS

#include <fstream>

#include <rocky/zagros/strategies/strategy.h>

namespace rocky{
namespace zagros{

/**
 * @brief base class for strategies for analysing loss functions
 * 
 * @tparam T_e 
 * @tparam T_dim 
 */
template<typename T_e, int T_dim>
class analysis_strategy: public basic_strategy<T_e, T_dim>{};

/**
 * @brief creating a 2d projection of loss function
 * 
 */
template<typename T_e, int T_dim>
class loss_projection_2d: public analysis_strategy<T_e, T_dim>{
protected:
    // objective system
    system<T_e>* problem_;
    // output directory path
    std::string output_path_;
    // loss boundaries
    T_e x_min_;
    T_e x_max_;
    T_e y_min_;
    T_e y_max_;
    // image size
    int width_;
    int height_;
    // resolution settings
    double delta_x_;
    double delta_y_;
    int l_width_;
    int l_height_;
    // step
    int step_;
    // loss function values
    std::vector<std::vector<T_e>> z;

public:
    loss_projection_2d(system<T_e>* problem, int width, int height, T_e x_min, T_e y_min, T_e x_max, T_e y_max){
        problem_ = problem;
        width_ = width;
        height_ = height;
        x_min_ = x_min;
        y_min_ = y_min;
        x_max_ = x_max;
        y_max_ = y_max;
        step_ = 0;
        // obtain resolution settings
        delta_x_ = (x_max_ - x_min_) / static_cast<double>(width_);
        delta_y_ = (y_max_ - y_min_) / static_cast<double>(height_);
        // real image size after possible numerical errors
        l_width_ = (x_max_ - x_min_) / delta_x_ ;
        l_height_ = (y_max_ - y_min_) / delta_y_;
        // allocate memory for the mesh
        z.resize(l_height_);
        for(int y=0; y<l_height_; y++)
            z[y].resize(l_width_);
    }
    void save_mesh(){
        std::string path = fmt::format("zagros_loss_{}.data", step_);
        std::fstream handler(path, std::fstream::out);
        // writing metadata
        handler << step_ << std::endl;
        handler << fmt::format("{} {} {} {}", x_min_, y_min_, x_max_, y_max_) << std::endl;
        handler << fmt::format("{} {}", l_height_, l_width_) << std::endl;
        // writing the mesh values
        for(int y=0; y<l_height_; y++)
            for(int x=0; x<l_width_; x++)
                handler << z[y][x] << " ";
        handler.close();
    }
    virtual void apply(){
        // traverse the 2D mesh in parallel
        tbb::parallel_for(0, this->l_width_, [&](auto x){
            tbb::parallel_for(0, this->l_height_, [&](auto y){
                T_e thread_point[2];
                thread_point[0] = x * this->delta_x_ + this->x_min_;
                thread_point[1] = y * this->delta_y_ + this->y_min_;
                this->z[y][x] = this->problem_->objective(thread_point);
            });
        });

        save_mesh();
        step_++;
    }
};

};
};

#endif