#ifndef CALLCENTRE_INCOMINGCALLSQUEUE_H
#define CALLCENTRE_INCOMINGCALLSQUEUE_H


#include <memory>
#include <unordered_map>
#include <list>
#include <condition_variable>

#include "Call.h"
#include "includes.h"
#include "Log.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


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

        void wait();

        void awake();

    private:

        void OnCallReset(const boost::system::error_code &ec, std::string cg_pn, ListIterator iter);


    private:
        net::io_context& ioc_;

        std::list<std::shared_ptr<Call>> incoming_calls_;

        std::vector<net::system_timer> timers_;

        // contains the indexes of the free timers in the vector
        std::queue<int> free_timers_;
        // сохраняет связь вызова в очереди с конкретным таймером и с позицией звонка в очереди(через итератор)
        // также служит для проверки дублирования звонка по его callID
        // key: CgPn, val: a pair of timers_ index and list iterator
        std::unordered_map<std::string, int> map_cg_pn_to_pos_;

        size_t capacity_;

        std::mutex mux_queue;

        std::condition_variable cv_blocking;
        std::mutex mux_blocking;

    };

}
#endif //CALLCENTRE_INCOMINGCALLSQUEUE_H
