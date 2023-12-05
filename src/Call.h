#ifndef CALLCENTRE_CALL_H
#define CALLCENTRE_CALL_H

#include <chrono>
#include <string>
#include <atomic>

#include "RandGen.h"


namespace call_c
{

    struct Call
    {
        enum class RespStatus {OK, OVERLOAD, TIMEOUT, ALREADY_IN_QUEUE};

        explicit Call(std::string phone_number);

        void SetOperatorResponseData(int op_id);

        // вызывается при ошибке или окончании звонка
        void SetCompleteData(RespStatus st);

        static uint32_t getNextCallId() { return next_call_id++; }

        // эти поля заполняются у любого звонка
        uint32_t call_id;
        std::string cg_pn;
        RespStatus status{};
        std::chrono::time_point<std::chrono::system_clock> dt_request;
        std::chrono::time_point<std::chrono::system_clock> dt_completion;

        // эти только у тех, что успешно провели звонок
        int operator_id{};
        std::chrono::time_point<std::chrono::system_clock> dt_response;

        // эти поля заполняются случайными значениями из RandGen при инициализации Call,
        std::chrono::milliseconds call_duration;
        std::chrono::milliseconds expiration_time;

    private:
        static std::atomic<uint32_t> next_call_id;

    };

    // нужны для вывода в логер date time полей из Call
    std::string tpToStrHMS(const std::chrono::time_point<std::chrono::system_clock> &tp);
    std::string tpToStrYMD(const std::chrono::time_point<std::chrono::system_clock> &tp);
}

#endif //CALLCENTRE_CALL_H
