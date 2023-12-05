#ifndef CALLCENTRE_OPERATORS_H
#define CALLCENTRE_OPERATORS_H

#include "includes.h"
#include "ThreadSafeQueue.h"
#include "Call.h"
#include "IncomingCallsQueue.h"

namespace net = boost::asio;            // from <boost/asio.hpp>


namespace call_c
{

    class Operators
    {
    public:
        Operators(net::io_context &ioc, int countOp, size_t incCallsSize);

        IncomingCallsQueue& getIncCalls() { return incoming_calls_; }

        void Run();

        void Stop();

    private:
        void AddCall();

        void OnEndCall(const boost::system::error_code &ec, const std::shared_ptr<Call>& call);

    private:
        IncomingCallsQueue incoming_calls_;

        std::vector<net::system_timer> operators_;

        tsqueue<int> free_operators_;

        std::atomic_bool continue_loop_ = true;
    };
}


#endif //CALLCENTRE_OPERATORS_H
