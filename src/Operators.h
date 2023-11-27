#ifndef CALLCENTRE_OPERATORS_H
#define CALLCENTRE_OPERATORS_H

#include "includes.h"
#include "ThreadSafeQueue.h"

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

        tsqueue<std::shared_ptr<Call>>& getIncCalls() { return incCalls_; }

        void run();

        void stop() {}

    private:

        void update();

        void add_call();

        void set_call_timer();

        void on_end_call(const boost::system::error_code &ec, std::shared_ptr<Call> call);

        void on_queue_expiry(const boost::system::error_code &ec, uint32_t expectedCallID);

    private:

        net::io_context &ioc_;
        net::strand<net::io_context::executor_type> strand_;

        net::system_timer timerIncCall_;
        tsqueue<std::shared_ptr<Call>> incCalls_;

        std::vector<net::system_timer> operators_;

        tsqueue<int> freeOps_;
    };
}


#endif //CALLCENTRE_OPERATORS_H
