#include <utility>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <fstream>

#include "src/includes.h"

#include "src/IncomingQueue.h"
#include "src/Call.h"
#include "src/handle_req_resp.h"
#include "src/RandGen.h"

#include "nlohmann/json.hpp"


namespace call_c
{
    class hi
    {
        int age;
        std::string name;
    public:
        hi(int age, std::string name) : age(age), name(std::move(name))
        {
        }

        hi() : age(0)
        {
            std::cout << "empty constructor\n";
        }

        hi(const hi &other)
        {
            age = other.age;
            name = other.name;
            std::cout << "COPY\n";
        }

        hi(hi &&other) noexcept
        {
            age = other.age;
            name = std::move(other.name);
            std::cout << "MOVE\n";
        }

        hi &operator=(const hi &other)
        {
            age = other.age;
            name = other.name;
            std::cout << "copy assignment operator\n";
            return *this;
        }

        hi &operator=(hi &&other) noexcept
        {
            age = other.age;
            name = std::move(other.name);
            std::cout << "move assignment operator\n";
            return *this;
        }

    };


    template<typename T>
    void qwe(const T &src, T &dst)
    {
        dst = std::move(src);
    }


    void make(tsqueue<std::shared_ptr<Call>> &queue, int id)
    {
        auto call = std::make_shared<Call>(id);
        queue.push_back(call);
    }


}

using json = nlohmann::json;


int main()
{

    std::ifstream configFile("configCallCentre.json");
    json data = json::parse(configFile);

    int count_operators = data["count_operators"];
    int incoming_queue_size = data["incoming_queue_size"];
    int time_in_queue = data["time_in_queue"];
    std::string log_server_level = data["log_server_level"];
    std::string log_operators_level = data["log_operators_level"];
    double rand_gen_erl_shape = data["rand_gen_erl_shape"];
    double rand_gen_erl_scale = data["rand_gen_erl_scale"];

    for (const auto& val : data.items())
    {
        std::cout << val.key() << val.value() << '\n';
    }

    return 0;
}