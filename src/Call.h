#ifndef CALLCENTRE_CALL_H
#define CALLCENTRE_CALL_H

#include <iostream>
#include <chrono>
#include <utility>
#include <string>


/**
 * сущность, описывающая свойства заявки, находящейся в обработке
 * данные отсюда будут использованы для записи в CDR(журнал)
 *
 * */

namespace call_c {

    struct Call {
        explicit Call(uint32_t id) : callID(id) {
            dt_req = std::chrono::system_clock::now();
        }

        Call() = default;

        // make a function get_cdr, either here or in the log logic

        uint32_t callID{};
        std::string CgPN{}; // now I use string, but in the future, string can be replaced with integral type
        uint16_t operatorID{};
        uint8_t status{}; // maybe enum
        std::chrono::time_point<std::chrono::system_clock> dt_req{};
        std::chrono::time_point<std::chrono::system_clock> dt_resp{};
        std::chrono::time_point<std::chrono::system_clock> dt_completion{};
        std::chrono::time_point<std::chrono::system_clock> dt_expiry{};

        // call duration is calculated -> dt_resp - dt_completion
    };

}

#endif //CALLCENTRE_CALL_H
