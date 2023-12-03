#include "RandGen.h"

namespace call_c
{
    thread_local std::mt19937 RandGen::gen_ = std::mt19937(std::random_device{}());
    std::gamma_distribution<> RandGen::erlang_dist_;
    std::uniform_real_distribution<double> RandGen::uniform_dist_;

    void RandGen::Init(double shape, double scale, double exp_time_min, double exp_time_max)
    {
        erlang_dist_ = std::gamma_distribution<>(shape, scale);
        uniform_dist_ = std::uniform_real_distribution<double>(exp_time_min, exp_time_max);
    }

    std::chrono::milliseconds RandGen::getRandErlang()
    {
        return std::chrono::milliseconds(static_cast<int>(1000 * erlang_dist_(gen_)));
    }

    std::chrono::milliseconds RandGen::getRandUniform()
    {
        return std::chrono::milliseconds(static_cast<int>(1000 * uniform_dist_(gen_)));
    }

    void RandGen::setRandErlang(double shape, double scale)
    {
        erlang_dist_ = std::gamma_distribution<>(shape, scale);
    }

    void RandGen::setRandUniform(double exp_time_min, double exp_time_max)
    {
        uniform_dist_ = std::uniform_real_distribution<double>(exp_time_min, exp_time_max);
    }
}