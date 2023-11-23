#ifndef CALLCENTRE_LISTENER_H
#define CALLCENTRE_LISTENER_H


#include "includes.h"
#include "Session.h"
#include "Log.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace call_c {

// Accepts incoming connections and launches the sessions
    class listener : public std::enable_shared_from_this<listener> {
    private:
        net::io_context &ioc_;
        tcp::acceptor acceptor_;
        uint32_t callId_{100};

        tsqueue<Call> &incCalls;

    public:
        listener(net::io_context &ioc, tcp::endpoint endpoint, tsqueue<Call> &incQueue)
                : ioc_(ioc), acceptor_(net::make_strand(ioc)), incCalls(incQueue) {
            beast::error_code ec;

            // open the acceptor
            acceptor_.open(endpoint.protocol(), ec);
            if (ec) {
                C_SERVER_ERROR("acceptor open: {}", ec.message());
                return;
            }

            // allow address reuse
            acceptor_.set_option(net::socket_base::reuse_address(true), ec);
            if (ec) {
                C_SERVER_ERROR("acceptor set_option: {}", ec.message());
                return;
            }

            // Bind to the server address
            acceptor_.bind(endpoint, ec);
            if (ec) {
                C_SERVER_ERROR("acceptor bind: {}", ec.message());
                return;
            }

            // Start listening for connections
            acceptor_.listen(net::socket_base::max_listen_connections, ec);
            if (ec) {
                C_SERVER_ERROR("acceptor listen: {}", ec.message());
                return;
            }
        }

        // Start accepting incoming connections
        void run() {
            C_SERVER_INFO("SERVER IS RUNNING");
            do_accept();
        }

    private:
        void do_accept() {
            // The new connection gets its own strand
            acceptor_.async_accept(
                    net::make_strand(ioc_),
                    beast::bind_front_handler(
                            &listener::on_accept,
                            shared_from_this()
                    )
            );
        }

        void on_accept(beast::error_code ec, tcp::socket socket) {
            C_SERVER_DEBUG("on_accept: {}:{} new connection", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());
            if (ec) {
                C_SERVER_ERROR("on_accept: {}", ec.message());
                return; // To avoid infinite loop
            } else {
                // Create the session and run it
                std::make_shared<session>(std::move(socket), incCalls, callId_++)
                        ->run();
            }

            // Accept another connection
            do_accept();
        }
    };

}

#endif //CALLCENTRE_LISTENER_H
