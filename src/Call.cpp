#include "Call.h"

namespace call_c
{
    std::atomic<uint32_t> Call::next_call_id = 100;

    Call::Call(std::string phone_number) : callID(getNextCallId()), CgPn(std::move(phone_number)),
                                           dt_req(std::chrono::system_clock::now()),
                                           call_duration(RandGen::getRandErlang()),
                                           expiration_time(RandGen::getRandUniform())
    {

    }

    void Call::SetOperatorResponseData(int OpId)
    {
        operatorID = OpId;
        dt_resp = std::chrono::system_clock::now();
        dt_completion = dt_resp + call_duration;
    }


    std::string tp_to_strHMS(const std::chrono::time_point<std::chrono::system_clock> &tp) {
        std::string s{};
        s.resize(70);
        std::time_t tp_c = std::chrono::system_clock::to_time_t(tp);
        auto num = std::strftime(s.data(), s.size(), "%T", std::localtime(&tp_c));
        s.resize(num);
        auto ms = time_point_cast<std::chrono::milliseconds>(tp).time_since_epoch().count() -
                  1000 * time_point_cast<std::chrono::seconds>(tp).time_since_epoch().count();
        s.push_back('.');
        std::string ms_s = std::to_string(ms);
        std::string ms_valid = "000";
        ms_valid.replace(ms_valid.size() - ms_s.size(), ms_s.size(), ms_s);
        s.append(ms_valid);
        return s;
    }

    std::string tp_to_strYMD(const std::chrono::time_point<std::chrono::system_clock> &tp)
    {
        std::string s{};
        s.resize(70);
        std::time_t tp_c = std::chrono::system_clock::to_time_t(tp);
        auto num = std::strftime(s.data(), s.size(), "%F", std::localtime(&tp_c));
        s.resize(num);
        return s;
    }

}