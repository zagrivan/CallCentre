#ifndef CALLCENTRE_INCOMINGCALLSQUEUE_H
#define CALLCENTRE_INCOMINGCALLSQUEUE_H


#include <memory>
#include <unordered_map>
#include <list>
#include <condition_variable>

#include "Call.h"
#include "includes.h"
#include "Log.h"

namespace net = boost::asio;            // from <boost/asio.hpp>

/*
 * Класс выполняет стандартные функции очереди и эмулирует сброс звонка во время ожидания ответа от оператора.
 * Под очередь выбрал std::list, чтобы можно было удобно удалять звонок при сбросе из любой части очереди.
 * Для эмуляции сброса используются boost::asio::system_timer.
 * К каждому звонку привязывается не занятый таймер из std::vector<net::system_timer> timers_ и ставится на рандомное время.
 * Если оператор успеет схватить звонок до истечения таймера, то таймер отменится,
 * иначе выполнится completion handler OnCallReset(...) и удалит звонок из очереди.
 */


namespace call_c
{

    class IncomingCallsQueue
    {
        using ListIterator = std::list<std::shared_ptr<Call>>::iterator;

    public:
        IncomingCallsQueue(size_t capacity, net::io_context& ioc);

        IncomingCallsQueue(const IncomingCallsQueue &) = delete; // non copied

        const std::shared_ptr<Call> &front();

        const std::shared_ptr<Call> &back();

        Call::RespStatus push(const std::shared_ptr<Call> &item);

        std::shared_ptr<Call> pop();

        bool empty();

        size_t size();

        void clear();

        // для ожидания появления звонка в операторах
        void wait();

        void awake();

    private:

        void OnCallReset(const boost::system::error_code &ec, std::string cg_pn, ListIterator iter);


    private:
        std::list<std::shared_ptr<Call>> incoming_calls_;

        std::vector<net::system_timer> timers_;

        // содержит индексы свободных таймеров
        std::queue<int> free_timers_;
        // сохраняет связь вызова в очереди с конкретным таймером
        // также служит для проверки дублирования звонка по его CgPn
        // key: CgPn, val: timers_ index
        std::unordered_map<std::string, int> map_cg_pn_to_pos_;

        size_t capacity_;

        std::mutex mux_queue;

        // используются в wait()
        std::condition_variable cv_blocking;
        std::mutex mux_blocking;

    };

}
#endif //CALLCENTRE_INCOMINGCALLSQUEUE_H
