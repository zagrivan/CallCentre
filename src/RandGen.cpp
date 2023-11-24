#include "RandGen.h"

namespace call_c
{
    std::mt19937 RandGen::gen;
    std::gamma_distribution<> RandGen::erlangDist;
    std::chrono::milliseconds RandGen::expirationTime;

    void RandGen::Init(double shape, double scale, double expTime)
    {
        std::random_device rd;
        gen = std::mt19937(rd());
        erlangDist = std::gamma_distribution<>(shape, scale);

        expirationTime = std::chrono::milliseconds(static_cast<int>(1000 * expTime));
    }
}