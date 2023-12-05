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
        Session(tcp::socket &&socket, IncomingCallsQueue& incoming_calls);
        // starts the asynchronous operation
        void Run();

    private:
        void DoRead();

        void OnRead(beast::error_code ec, std::size_t bytes_transferred);

        void SendResponse(http::message_generator &&msg);

        void OnWrite(beast::error_code ec, std::size_t bytes_transferred);

        void DoClose();

        http::message_generator HandleRequest();

        http::message_generator AddToQueue(const std::shared_ptr<Call>& call);

    private:
        beast::tcp_stream stream_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        IncomingCallsQueue& incoming_calls_;
    };

    http::message_generator HandleValidReq(uint http_version, const std::string& message);
    http::message_generator HandleBadReq(uint http_version, const std::string& message);
    bool isValidPhoneNumber(std::string query_string, std::string& prepared_phone_number);
}

#endif //CALLCENTRE_SESSION_H
