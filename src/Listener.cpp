#include "Listener.h"
#include "Session.h"
#include "Log.h"


namespace call_c
{
    Listener::Listener(net::io_context &ioc, tcp::endpoint endpoint)
            : ioc_(ioc), acceptor_(net::make_strand(ioc))
    {
        beast::error_code ec;
        // open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec)
        {
            LOG_SERVER_ERROR("acceptor open: {}", ec.message());
            return;
        }

        // allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec)
        {
            LOG_SERVER_ERROR("acceptor set_option: {}", ec.message());
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec)
        {
            LOG_SERVER_ERROR("acceptor bind: {}", ec.message());
            return;
        }

        // Start listening for connections
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec)
        {
            LOG_SERVER_ERROR("acceptor listen: {}", ec.message());
            return;
        }
    }

    // Start accepting incoming connections
    void Listener::run() {
        LOG_SERVER_INFO("SERVER IS RUNNING");
        do_accept();
    }

    void Listener::do_accept() {
        // The new connection gets its own strand
        acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                        &Listener::on_accept,
                        shared_from_this()
                )
        );
    }

    void Listener::on_accept(beast::error_code ec, tcp::socket socket) {
        LOG_SERVER_DEBUG("on_accept: {}:{} new connection", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());
        if (ec) {
            LOG_SERVER_ERROR("on_accept: {}", ec.message());
            return; // To avoid infinite loop
        } else {
            // Create the session and run it
            std::make_shared<Session>(std::move(socket))
                    ->run();
        }
        // Accept another connection
        do_accept();
    }

}