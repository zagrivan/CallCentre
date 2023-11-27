#ifndef CALLCENTRE_READJSON_H
#define CALLCENTRE_READJSON_H

#include "nlohmann/json.hpp"
#include <fstream>

namespace call_c
{

    using json = nlohmann::json;

    class ReadJSON
    {
    public:
        ReadJSON(const std::string& configFileName)
        {
            std::ifstream f{configFileName};
            json data = json::parse(f);

            cntOperators = data["count_operators"];
            incQueueSize = data["incoming_queue_size"];
            timeInQueue = data["time_in_queue"];
            logServerLevel = data["log_server_level"];
            logOperatorsLevel = data["log_operators_level"];
            randGenErlShape = data["rand_gen_erl_shape"];
            randGenErlScale = data["rand_gen_erl_scale"];
        }

    public:
        int cntOperators;
        size_t incQueueSize;
        int timeInQueue;
        std::string logServerLevel;
        std::string logOperatorsLevel;
        double randGenErlShape;
        double randGenErlScale;
    };
}

#endif //CALLCENTRE_READJSON_H
