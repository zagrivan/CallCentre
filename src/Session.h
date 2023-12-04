#ifndef CALLCENTRE_SESSION_H
#define CALLCENTRE_SESSION_H


#include "includes.h"
#include "Call.h"
#include "IncomingCallsQueue.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace call_c {

    // Handles an HTTP server connection
    class Session : public std::enable_shared_from_this<Session> {

    public:
        // Take ownership of the stream
        explicit Session(tcp::socket &&socket);
        // starts the asynchronous operation
        void run();

        static void setIncomingCallsQueue(IncomingCallsQueue* incomingCalls) { incomingCalls_ = incomingCalls; }
    private:
        void do_read();

        void on_read(beast::error_code ec, std::size_t bytes_transferred);

        void send_response(http::message_generator &&msg);

        void on_write(beast::error_code ec, std::size_t bytes_transferred);

        void do_close();

        http::message_generator handle_request();

        http::message_generator AddToQueue(std::shared_ptr<Call> call);

    private:
        beast::tcp_stream stream_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        // TODO: безопасно ли?
        static IncomingCallsQueue* incomingCalls_;
    };

    http::message_generator handle_valid_req(uint http_version, const std::string& message);
    http::message_generator handle_bad_req(uint http_version, const std::string &why);
    bool isValidPhoneNumber(std::string query_string, std::string& prepared_phone_number);
}

#endif //CALLCENTRE_SESSION_H
