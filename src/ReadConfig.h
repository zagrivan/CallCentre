#ifndef CALLCENTRE_READCONFIG_H
#define CALLCENTRE_READCONFIG_H

#include "nlohmann/json.hpp"
#include <fstream>

namespace call_c
{

    using json = nlohmann::json;

    class ReadConfig
    {
    public:
        explicit ReadConfig(const std::string& configFileName)
        {
            std::ifstream f{configFileName};
            data_ = json::parse(f);
            ReadJson();
        }

    private:
        void ReadJson()
        {
            count_operators = data_["count_operators"];
            incoming_queue_size = data_["incoming_queue_size"];
            log_server_level = data_["log"]["server_level"];
            log_operators_level = data_["log"]["operators_level"];
            timeout_in_queue_min = data_["rand_timeout_in_queue"]["rand_uniform_min"];
            timeout_in_queue_max = data_["rand_timeout_in_queue"]["rand_uniform_max"];
            rand_gen_erl_shape = data_["rand_call_duration"]["rand_gen_erl_shape"];
            rand_gen_erl_scale = data_["rand_call_duration"]["rand_gen_erl_scale"];
        }

    public:
        int count_operators{};
        size_t incoming_queue_size{};
        std::string log_server_level;
        std::string log_operators_level;
        double rand_gen_erl_shape{-1};
        double rand_gen_erl_scale{-1};
        double timeout_in_queue_min{-1};
        double timeout_in_queue_max{-1};

    private:
        json data_;
    };
}

#endif //CALLCENTRE_READCONFIG_H
