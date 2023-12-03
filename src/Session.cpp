#include "Session.h"
#include "Log.h"
#include "handle_req_resp.h"

namespace call_c
{
    IncomingCallsQueue* Session::incomingCalls_{nullptr};

    Session::Session(tcp::socket &&socket)
            : stream_(std::move(socket))
    {
        LOG_SERVER_DEBUG("{} New HTTP connection", stream_.socket().remote_endpoint().address().to_string());
    }

    void Session::run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(
                stream_.get_executor(),
                beast::bind_front_handler(&Session::do_read, shared_from_this())
        );
    }

    void Session::do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout
        stream_.expires_after(std::chrono::seconds(10));

        // Read a request
        http::async_read(stream_, buffer_, req_,
                         beast::bind_front_handler(&Session::on_read, shared_from_this()));
    }

    void Session::on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        // This means they closed the connection
        if (ec == http::error::end_of_stream)
            return do_close();

        if (ec)
        {
            LOG_SERVER_ERROR("READ FROM SOCKET ERROR", ec.message());
            return;
        }
        // Send the response
        // здесь обрабатывается http request
        send_response(handle_request());
    }

    void Session::send_response(http::message_generator &&msg)
    {
        // Write the response
        beast::async_write(
                stream_,
                std::move(msg),
                beast::bind_front_handler(&Session::on_write, shared_from_this())
        );
    }

    void Session::on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        if (ec)
        {
            LOG_SERVER_ERROR("WRITE TO SOCKET ERROR {}", ec.message());
            return;
        }
        do_close();
    }

    void Session::do_close()
    {
        // send a tcp shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec)
        {
            LOG_SERVER_ERROR("SOCKET SHUTDOWN ERROR {}", ec.message());
            return;
        }

        LOG_SERVER_DEBUG("{} connection closed successfully",
                         stream_.socket().remote_endpoint().address().to_string());
    }

    http::message_generator Session::handle_request()
    {
        if (req_.method() != http::verb::get)
        {
            LOG_SERVER_INFO("Invalid request: был использован не GET request IP:{}", stream_.socket().remote_endpoint().address().to_string());
            return handle_bad_req(req_.version(), "Usage GET method.");
        }
        std::string cg_pn{};
        if (!isValidPhoneNumber(req_.target(), cg_pn))
        {
            LOG_SERVER_INFO("Invalid request: неправильно набран номер: {} IP:{}", req_.target(), stream_.socket().remote_endpoint().address().to_string());
            return handle_bad_req(req_.version(), "Invalid phone number.");
        }

        auto call = std::make_shared<Call>(cg_pn);

        return AddToQueue(call);
    }

    http::message_generator Session::AddToQueue(std::shared_ptr<Call> call)
    {
        // добавляем в очередь
        Call::RespStatus status = incomingCalls_->push(call);
        std::string message;

        switch (status)
        {
            case Call::RespStatus::OK:
                message.append("CallId: ").append(std::to_string(call->callID));
                return handle_valid_req(req_.version(), message);
            case Call::RespStatus::OVERLOAD:
                message.append("The queue is overloaded");
                call->status = Call::RespStatus::OVERLOAD;
                break;
            case Call::RespStatus::ALREADY_IN_QUEUE:
                message.append("This phone number is already waiting");
                call->status = Call::RespStatus::ALREADY_IN_QUEUE;
                break;
        }

        call->dt_completion = std::chrono::system_clock::now();
        Log::WriteCDR(call);
        return handle_valid_req(req_.version(), message);
    }

}