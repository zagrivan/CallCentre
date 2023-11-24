#include "includes.h"

#include <boost/asio/signal_set.hpp>

#include "Session.h"
#include "Listener.h"
#include "Call.h"
#include "IncomingQueue.h"
#include "Operators.h"
#include "Log.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4)
    {
        std::cerr <<
                  "Usage: http-server-async <address> <port> <threads>\n" <<
                  "Example:\n" <<
                  "    http-server-async 0.0.0.0 8080 1\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    net::io_context ioc{};

    call_c::Operators operators{ioc, 5, 5, 5, 5, 10};

    call_c::Log::Init();

    // Create and launch a listening port
    std::make_shared<call_c::listener>(
            ioc,
            tcp::endpoint{address, port},
            operators.getIncCalls())->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&](beast::error_code const&, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                ioc.stop();
            });

    std::thread thrForOperators{
        [&operators] {
            operators.run();
        }
    };

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
    ioc.run();

    for (auto& t : v)
        t.join();

    // TODO останавливаем операторов
    operators.stop();
    thrForOperators.detach();

    return EXIT_SUCCESS;
}