#ifndef CALLCENTRE_RANDGEN_H
#define CALLCENTRE_RANDGEN_H

#include <random>
#include <chrono>

namespace call_c
{
    class RandGen
    {
    public:
        static void Init(double shape, double scale, double expTime);

        static std::chrono::milliseconds getRandErlang()
        {
//            return std::chrono::milliseconds(static_cast<int>(1000 * erlangDist(gen)));
            return std::chrono::milliseconds(static_cast<int>(1000 * erlangDist(gen)));
        }

        static std::chrono::milliseconds getExpirationTime() {
            return expirationTime;
        }

    private:
        static std::mt19937 gen;
        static std::gamma_distribution<> erlangDist;
        static std::chrono::milliseconds expirationTime;
    };
}

#endif //CALLCENTRE_RANDGEN_H
