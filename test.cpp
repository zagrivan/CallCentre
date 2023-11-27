#include <utility>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <fstream>

#include "src/includes.h"

#include "src/ThreadSafeQueue.h"
#include "src/Call.h"
#include "src/handle_req_resp.h"
#include "src/RandGen.h"
#include "src/Log.h"

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

    using namespace call_c;

    Log::Init("trace", "trace");

    LOG_OPERATORS_CRITICAL("SLKADFJSLK");
    LOG_SERVER_CRITICAL("SLKADFJSLK");

    LOG_OPERATORS_INFO("sdfklj js");
    LOG_OPERATORS_DEBUG("asdlkjfjdsg dfs g");

    LOG_SERVER_INFO("sdfklj js");
    LOG_SERVER_DEBUG("asdlkjfjdsg dfs g");

    return 0;
}