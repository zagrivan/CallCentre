#include <utility>

#include "src/includes.h"

#include "src/IncomingQueue.h"
#include "src/Call.h"

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
    using namespace call_c;

    tsqueue <std::shared_ptr<Call>> ts(5);

    ts.push_back(std::make_shared<Call>(101));
    ts.pop_front();

    auto ptr = ts.front();
    std::cout << ptr;

    return 0;
}