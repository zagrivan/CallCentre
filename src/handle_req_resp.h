#ifndef CALLCENTRE_HANDLE_REQ_RESP_H
#define CALLCENTRE_HANDLE_REQ_RESP_H

#include "includes.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace call_c {

// Return a response for the given request.
//template<typename Body, typename Allocator>
//http::message_generator
//handle_request(http::request<Body, http::basic_fields<Allocator>>&& req, uint32_t CallID)
    http::message_generator handle_ok(uint http_version, uint32_t CallID) {
        // Respond to GET request
        http::response<http::string_body> res{http::status::ok, http_version};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(false);
        res.body() = std::to_string(CallID);
        res.prepare_payload();

        return res;
    }

    http::message_generator handle_bad_req(uint http_version, const std::string &why) {
        http::response<http::string_body> res{http::status::bad_request, http_version};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(false);
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    }


    http::message_generator handle_queue_overload(uint http_version) {
        http::response<http::string_body> res{http::status::internal_server_error, http_version};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(false);
        res.body() = "the queue is overloaded";
        res.prepare_payload();
        return res;
    }


// Report a failure
    template<typename EC>
    void fail(EC ec, char const *what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }


    std::string tp_to_strHMS(const std::chrono::time_point<std::chrono::system_clock> &tp) {
        std::string s{};
        s.resize(70);
        std::time_t tp_c = std::chrono::system_clock::to_time_t(tp);
        auto num = std::strftime(s.data(), s.size(), "%T", std::localtime(&tp_c));
        s.resize(num);
        return s;
    }

}

#endif //CALLCENTRE_HANDLE_REQ_RESP_H
