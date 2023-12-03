#include "handle_req_resp.h"

namespace call_c {

// Return a response for the given request.
//template<typename Body, typename Allocator>
//http::message_generator
//handle_request(http::request<Body, http::basic_fields<Allocator>>&& req, uint32_t CallID)

    http::message_generator handle_valid_req(uint http_version, const std::string& message) {
        // Respond to GET request
        http::response<http::string_body> res{http::status::ok, http_version};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(false);
        res.body() = message;
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

    void getPhoneNumber(const std::smatch & matches, std::string& valid_phone_number)
    {
        int i = 1;
        if (!matches[1].length() || (matches[1].compare("8") == 0))
        {
            valid_phone_number.append("7");
            i = 2;
        }
        for (; i != matches.size(); ++i)
        {
            valid_phone_number.append(matches[i].str());
        }
    }

    void UrlDecode(std::string& str)
    {
        size_t start_pos = 0;
        const std::string from = "%20";
        const std::string to = " ";
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
    }

    bool isValidPhoneNumber(std::string query_string, std::string& prepared_phone_number)
    {
        UrlDecode(query_string);
        const std::regex pattern(R"(^/\s*(?:\+?(\d{1,3}))?[-. (]*(\d{3})[-. )]*(\d{3})[-. ]*(\d{4})\s*$)");
        std::smatch matches;

        if (!std::regex_match(query_string, matches, pattern)) {
            return false;
        }

        getPhoneNumber(matches, prepared_phone_number);

        return true;
    }
}