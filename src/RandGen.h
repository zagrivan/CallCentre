#ifndef CALLCENTRE_RANDGEN_H
#define CALLCENTRE_RANDGEN_H

#include <random>
#include <chrono>

namespace call_c
{
    class RandGen
    {
    public:
        static void Init(double shape, double scale, double exp_time_min, double exp_time_max);

        static std::chrono::milliseconds getRandErlang();

        static std::chrono::milliseconds getRandUniform();

        static void setRandErlang(double shape, double scale);

        static void setRandUniform(double exp_time_min, double exp_time_max);

        static double erlang_dist_shape() { return erlang_dist_.alpha();}
        static double erlang_dist_scale() { return erlang_dist_.beta();}
        static double uniform_dist_min() { return uniform_dist_.a();}
        static double uniform_dist_max() { return uniform_dist_.b();}

    private:
        static thread_local std::mt19937 gen_;

        static std::gamma_distribution<> erlang_dist_;

        static std::uniform_real_distribution<double> uniform_dist_;
    };
}

#endif //CALLCENTRE_RANDGEN_H
