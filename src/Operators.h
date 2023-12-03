#ifndef CALLCENTRE_OPERATORS_H
#define CALLCENTRE_OPERATORS_H

#include "includes.h"
#include "ThreadSafeQueue.h"
#include "Call.h"
#include "IncomingCallsQueue.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace call_c
{

    class Operators
    {
        using time_point = std::chrono::time_point<std::chrono::system_clock>;

    public:
        Operators(net::io_context &ioc, int countOp, size_t incCallsSize);

        IncomingCallsQueue& getIncCalls() { return incCalls_; }

        void run();

        void stop();

    private:
        void add_call();

        void on_end_call(const boost::system::error_code &ec, std::shared_ptr<Call> call);

    private:

        net::io_context &ioc_;

        IncomingCallsQueue incCalls_;

        std::vector<net::system_timer> operators_;

        tsqueue<int> freeOps_;

        std::atomic_bool continue_loop_ = true;
    };
}


#endif //CALLCENTRE_OPERATORS_H
