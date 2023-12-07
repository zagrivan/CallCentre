#include <regex>

#include "Session.h"
#include "Log.h"


namespace call_c
{

    Session::Session(tcp::socket &&socket, IncomingCallsQueue& incoming_calls)
            : stream_(std::move(socket)), incoming_calls_(incoming_calls)
    { }

    void Session::Run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(
                stream_.get_executor(),
                beast::bind_front_handler(&Session::DoRead, shared_from_this())
        );
    }

    void Session::DoRead()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout
        stream_.expires_after(std::chrono::seconds(10));

        // Read a request
        http::async_read(stream_, buffer_, req_,
                         beast::bind_front_handler(&Session::OnRead, shared_from_this()));
    }

    void Session::OnRead(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        // This means they closed the connection
        if (ec == http::error::end_of_stream)
            return DoClose();

        if (ec)
        {
            LOG_SERVER_ERROR("READ FROM SOCKET ERROR", ec.message());
            return;
        }
        // Send the response
        // здесь обрабатывается http request
        SendResponse(HandleRequest());
    }

    void Session::SendResponse(http::message_generator &&msg)
    {
        // Write the response
        beast::async_write(
                stream_,
                std::move(msg),
                beast::bind_front_handler(&Session::OnWrite, shared_from_this())
        );
    }

    void Session::OnWrite(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        if (ec)
        {
            LOG_SERVER_ERROR("WRITE TO SOCKET ERROR {}", ec.message());
            return;
        }
        DoClose();
    }

    void Session::DoClose()
    {
        // send a tcp shutdown
        beast::error_code ec;

        stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec)
        {
            LOG_SERVER_ERROR("SOCKET SHUTDOWN ERROR {}", ec.message());
            return;
        }
    }

    http::message_generator Session::HandleRequest()
    {
        if (req_.method() != http::verb::get)
        {
            LOG_SERVER_INFO("Invalid request: no GET request IP:{}", stream_.socket().remote_endpoint().address().to_string());
            return HandleBadReq(req_.version(), "Usage GET method.");
        }

        std::string cg_pn{}; // для обработанного номера телефона, запишется в isValidPhoneNumber, если номер валидный
        if (!isValidPhoneNumber(req_.target(), cg_pn))
        {
            LOG_SERVER_INFO("Invalid phone number: the number was incorrectly dialed: {} IP:{}", req_.target(), stream_.socket().remote_endpoint().address().to_string());
            return HandleBadReq(req_.version(), "Invalid phone number.");
        }

        auto call = std::make_shared<Call>(cg_pn);

        return AddToQueue(call);
    }

    http::message_generator Session::AddToQueue(const std::shared_ptr<Call>& call)
    {
        Call::RespStatus status = incoming_calls_.push(call);

        // это сообщение будет отправлено клиенту
        std::string message;

        switch (status)
        {
            case Call::RespStatus::OK:                   // отправляем в ответ просто call id
                LOG_OPERATORS_INFO("CallID:{} CgPn:{} WAIT IN QUEUE", call->call_id, call->cg_pn);
                message.append("CallId: ").append(std::to_string(call->call_id));
                return HandleValidReq(req_.version(), message);
            case Call::RespStatus::OVERLOAD:
                LOG_OPERATORS_INFO("CallID:{} CgPn:{} QUEUE OVERLOAD", call->call_id, call->cg_pn);
                message.append("The queue is overloaded");
                call->SetCompleteData(Call::RespStatus::OVERLOAD);
                break;
            case Call::RespStatus::ALREADY_IN_QUEUE:
                LOG_OPERATORS_INFO("CallID:{} CgPn:{} ALREADY IN QUEUE", call->call_id, call->cg_pn);
                message.append("This phone number is already waiting");
                call->SetCompleteData(Call::RespStatus::ALREADY_IN_QUEUE);
                break;
        }

        Log::WriteCDR(call);
        return HandleValidReq(req_.version(), message);
    }


    // non-member functions

    http::message_generator HandleValidReq(uint http_version, const std::string& message) {
        // Respond to GET request
        http::response<http::string_body> res{http::status::ok, http_version};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(false);
        res.body() = message;
        res.prepare_payload();

        return res;
    }

    // неправильный ввод символов или номера в url после слэша расценивается как 404
    http::message_generator HandleBadReq(uint http_version, const std::string& why) {
        http::response<http::string_body> res{http::status::bad_request, http_version};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(false);
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    }


    void PreparePhoneNumber(const std::smatch & matches, std::string& valid_phone_number)
    {
        int i = 1;
        // если код страны пустой или равен 8, то ставим 7
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

    // меняет %20 на пробелы
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
        // удаляем пробелы
        UrlDecode(query_string);
        // под код страны одна цифра, код страны не обязателен
        const std::regex pattern(R"(^/\s*(?:\+?(\d{1}))?[-. (]*(\d{3})[-. )]*(\d{3})[-. ]*(\d{4})\s*$)");
        std::smatch matches;

        if (!std::regex_match(query_string, matches, pattern)) {
            return false;
        }
        // приводим номер к общему формату
        PreparePhoneNumber(matches, prepared_phone_number);

        return true;
    }

}