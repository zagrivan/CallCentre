#ifndef CALLCENTRE_OPERATORS_H
#define CALLCENTRE_OPERATORS_H

#include "includes.h"
#include "ThreadSafeQueue.h"
#include "Call.h"
#include "IncomingCallsQueue.h"

namespace net = boost::asio;            // from <boost/asio.hpp>

/**
 * Под операторов выделяется отдельный поток, в котором запускается Operators::Run()
 * Вся логика заключается в том, что как только звонок приходит в очередь,
 * он отдается свободному оператору из std::vector<net::system_timer> operators_
 * Как таковых операторов нет, есть net::system_timer, который эмулирует разговор.
 * Таймер асинхронно срабатывает через call->call_duration секунд, вызывая Operators::OnEndCall(...), как обработчик завершения,
 * который запишет вызов в CDR, как успешно завершенный и пометит оператора как свободного,
 * добавив индекс свободного таймера в векторе в tsqueue<int> free_operators_.
 */

namespace call_c
{

    class Operators
    {
    public:
        Operators(net::io_context &ioc, int count_op, size_t incCallsSize);

        IncomingCallsQueue& getIncCalls() { return incoming_calls_; }

        void Run();

        void Stop();

    private:
        void AddCall();

        void OnEndCall(const boost::system::error_code &ec, const std::shared_ptr<Call>& call);

    private:
        IncomingCallsQueue incoming_calls_;

        std::vector<net::system_timer> operators_;

        TSQueue<int> free_operators_;

        std::atomic_bool continue_loop_ = true;
    };
}


#endif //CALLCENTRE_OPERATORS_H
