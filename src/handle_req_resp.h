#ifndef CALLCENTRE_HANDLE_REQ_RESP_H
#define CALLCENTRE_HANDLE_REQ_RESP_H

#include "includes.h"
#include <regex>


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace call_c {

// Return a response for the given request.
//template<typename Body, typename Allocator>
//http::message_generator
//handle_request(http::request<Body, http::basic_fields<Allocator>>&& req, uint32_t CallID)

    http::message_generator handle_valid_req(uint http_version, const std::string& message);

    http::message_generator handle_bad_req(uint http_version, const std::string &why);

    bool isValidPhoneNumber(std::string query_string, std::string& prepared_phone_number);
}

#endif //CALLCENTRE_HANDLE_REQ_RESP_H
