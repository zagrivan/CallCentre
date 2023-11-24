#include <utility>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>

#include "src/includes.h"

#include "src/IncomingQueue.h"
#include "src/Call.h"
#include "src/handle_req_resp.h"
#include "src/RandGen.h"


namespace call_c {
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


    void make(tsqueue <std::shared_ptr<Call>> &queue, int id)
    {
        auto call = std::make_shared<Call>(id);
        queue.push_back(call);
    }


}


int main()
{

    using namespace std::chrono;
    using namespace call_c;

    RandGen::Init(1, 10, 5);

    std::vector<milliseconds> v{};

    for (int i = 0; i != 10; ++i)
    {
        v.push_back(RandGen::getRandErlang());
    }

    std::sort(v.begin(), v.end());

    for (auto val: v)
    {
        std::cout << val.count() << ' ';
    }

    return 0;
}