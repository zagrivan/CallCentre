#include "Call.h"


namespace call_c
{
    std::atomic<uint32_t> Call::next_call_id = 100;

    Call::Call(std::string phone_number)
            : call_id(getNextCallId()), cg_pn(std::move(phone_number)), dt_request(std::chrono::system_clock::now()),
              call_duration(RandGen::getRandErlang()), expiration_time(RandGen::getRandUniform())
    {}

    void Call::SetOperatorResponseData(int op_id)
    {
        operator_id = op_id;
        dt_response = std::chrono::system_clock::now();
    }

    void Call::SetCompleteData(RespStatus st)
    {
        status = st;
        dt_completion = std::chrono::system_clock::now();
    }


    std::string tpToStrHMS(const std::chrono::time_point<std::chrono::system_clock> &tp) {
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

    std::string tpToStrYMD(const std::chrono::time_point<std::chrono::system_clock> &tp)
    {
        std::string s{};
        s.resize(70);
        std::time_t tp_c = std::chrono::system_clock::to_time_t(tp);
        auto num = std::strftime(s.data(), s.size(), "%F", std::localtime(&tp_c));
        s.resize(num);
        return s;
    }

}