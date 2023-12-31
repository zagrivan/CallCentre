#ifndef CALLCENTRE_LISTENER_H
#define CALLCENTRE_LISTENER_H


#include "includes.h"
#include "Call.h"
#include "IncomingCallsQueue.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace call_c
{

// Accepts incoming connections and launches the sessions
    class Listener : public std::enable_shared_from_this<Listener>
    {

    public:
        Listener(net::io_context &ioc, tcp::endpoint endpoint, IncomingCallsQueue& incoming_calls);

        // Start accepting incoming connections
        void run();

    private:
        void do_accept();

        void on_accept(beast::error_code ec, tcp::socket socket);

    private:
        net::io_context &ioc_;
        tcp::acceptor acceptor_;
        // для передачи объекту Session, когда придет запрос
        IncomingCallsQueue& incoming_calls_;
    };

}

#endif //CALLCENTRE_LISTENER_H
