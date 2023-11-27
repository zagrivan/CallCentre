#ifndef CALLCENTRE_SESSION_H
#define CALLCENTRE_SESSION_H


/* TODO
    возможно сделать callid генератор здесь, статик полем
*/


#include "includes.h"
#include "handle_req_resp.h"
#include "Call.h"
#include "IncomingQueue.h"
#include "Log.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace call_c {

// Handles an HTTP server connection
    class Session : public std::enable_shared_from_this<Session> {

    public:
        // Take ownership of the stream
        Session(tcp::socket &&socket, tsqueue<std::shared_ptr<Call>> &queue, uint32_t callID);
        // starts the asynchronous operation
        void run();

    private:
        void do_read();
        // TODO здесь я буду ставить номер в очередь
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        // TODO здесь буду отправлять CallID обратно, либо ставить ошибку и уведомлять о ней
        void send_response(http::message_generator &&msg);

        void on_write(beast::error_code ec, std::size_t bytes_transferred);

        void do_close();

        http::message_generator handle_request();

    private:
        beast::tcp_stream stream_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        uint32_t CallID_;
        std::shared_ptr<Call> call;

        tsqueue<std::shared_ptr<Call>> &incomingCalls;
    };

}

#endif //CALLCENTRE_SESSION_H
