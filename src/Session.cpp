#include "Session.h"


namespace call_c
{
    Session::Session(tcp::socket &&socket, tsqueue<std::shared_ptr<Call>> &queue, uint32_t callID)
            : stream_(std::move(socket)), CallID_(callID), incomingCalls(queue)
    {
        call = std::make_shared<Call>(callID);
        C_SERVER_DEBUG("{} CALLID:{} New HTTP connection", stream_.socket().remote_endpoint().address().to_string(),
                       callID);
    }

    void Session::run() {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(
                stream_.get_executor(),
                beast::bind_front_handler(&Session::do_read, shared_from_this())
        );
    }

    void Session::do_read() {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout
        stream_.expires_after(std::chrono::seconds(10));

        // Read a request
        http::async_read(stream_, buffer_, req_,
                         beast::bind_front_handler(&Session::on_read, shared_from_this()));
    }

    void Session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        // This means they closed the connection
        if (ec == http::error::end_of_stream)
            return do_close();

        if (ec) {
            C_SERVER_ERROR("READ FROM SOCKET ERROR {}, CALLID:{}", ec.message(), CallID_);
            return;
        }
        C_SERVER_DEBUG("reading from socket was successful, CALLID:{}", CallID_);
        // Send the response
        send_response(handle_request());
    }

    void Session::send_response(http::message_generator &&msg) {
        // Write the response
        beast::async_write(
                stream_,
                std::move(msg),
                beast::bind_front_handler(&Session::on_write, shared_from_this())
        );
    }

    void Session::on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (ec) {
            C_SERVER_ERROR("WRITE TO SOCKET ERROR {}, CALLID:{}", ec.message(), CallID_);
            return;
        }
        C_SERVER_DEBUG("writing to socket was successful, CALLID:{}", CallID_);
        do_close();
    }

    void Session::do_close() {
        // send a tcp shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec)
        {
            C_SERVER_ERROR("SOCKET SHUTDOWN ERROR {}, CALLID:{}", ec.message(), CallID_);
            return;
        }

        C_SERVER_DEBUG("{}, CALLID:{} connection closed successfully", stream_.socket().remote_endpoint().address().to_string(), CallID_);
    }

    http::message_generator Session::handle_request() {
        if (req_.method() != http::verb::get)
            return handle_bad_req(req_.version(), "Usage GET method");
        if (req_.target()[0] != '/' || req_.target().size() < 3)
            return handle_bad_req(req_.version(), "Invalid URI");

        auto cgpn = req_.target().substr(1);
        for (auto c: cgpn) {
            if (!std::isdigit(c))
                return handle_bad_req(req_.version(), "Invalid number");
        }

        // cgpn прошел валидацию, можем присвоить
        call->CgPN = cgpn;

        C_SERVER_INFO("CALLID:{}, CgPn:{} passed validation successfully", CallID_, cgpn);
        // push_back() to queue
        // if push_back to queue was successful, we send CallID
        // TODO можно поменять на try catch
        if (!incomingCalls.push_back(call)) {
            C_SERVER_WARN("CALLID:{}, CgPn:{} not added to queue, queue is overload", CallID_, cgpn);
            return handle_queue_overload(req_.version());
        }

        C_SERVER_DEBUG("CALLID:{}, CgPn:{} added to queue", CallID_, cgpn);
        return handle_ok(req_.version(), CallID_);
    }



}