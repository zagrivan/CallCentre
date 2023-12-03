#ifndef CALLCENTRE_CALL_H
#define CALLCENTRE_CALL_H

#include <iostream>
#include <chrono>
#include <utility>
#include <string>

#include "RandGen.h"

/**
 * сущность, описывающая свойства заявки, находящейся в обработке
 * данные отсюда будут использованы для записи в CDR(журнал)
 *
 * */

namespace call_c
{

    struct Call
    {
        enum class RespStatus {OK, OVERLOAD, TIMEOUT, ALREADY_IN_QUEUE};

        explicit Call(std::string phone_number);

        void SetOperatorResponseData(int OpId);

        uint32_t callID;
        std::string CgPn; // now I use string, but in the future, string can be replaced with integral type
        RespStatus status{};
        std::chrono::time_point<std::chrono::system_clock> dt_req;
        std::chrono::time_point<std::chrono::system_clock> dt_completion{};

        int operatorID{};
        std::chrono::time_point<std::chrono::system_clock> dt_resp{};
        std::chrono::milliseconds call_duration;

        std::chrono::milliseconds expiration_time;
//        std::chrono::time_point<std::chrono::system_clock> dt_expiry{};

        static uint32_t getNextCallId()
        {
            return next_call_id++;
        }

    private:
        static uint32_t next_call_id;
    };


    std::string tp_to_strHMS(const std::chrono::time_point<std::chrono::system_clock> &tp);
    std::string tp_to_strYMD(const std::chrono::time_point<std::chrono::system_clock> &tp);

}

#endif //CALLCENTRE_CALL_H
