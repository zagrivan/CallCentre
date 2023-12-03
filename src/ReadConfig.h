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
            json data = json::parse(f);

            count_operators = data["count_operators"];
            incoming_queue_size = data["incoming_queue_size"];
            timeout_in_queue_min = data["timeout_in_queue_min"];
            timeout_in_queue_max = data["timeout_in_queue_max"];
            log_server_level = data["log_server_level"];
            log_operators_level = data["log_operators_level"];
            rand_gen_erl_shape = data["rand_gen_erl_shape"];
            rand_gen_erl_scale = data["rand_gen_erl_scale"];
        }

    public:
        int count_operators;
        size_t incoming_queue_size;
        std::string log_server_level;
        std::string log_operators_level;
        double rand_gen_erl_shape;
        double rand_gen_erl_scale;
        double timeout_in_queue_min;
        double timeout_in_queue_max;
    };
}

#endif //CALLCENTRE_READCONFIG_H
