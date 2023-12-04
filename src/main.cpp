#include "includes.h"

#include <boost/asio/signal_set.hpp>

#include "Session.h"
#include "Listener.h"
#include "Call.h"
#include "Log.h"
#include "Operators.h"
#include "RandGen.h"
#include "ReadConfig.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


int main(int argc, char *argv[])
{
    // Check command line arguments.
    if (argc != 4)
    {
        std::cerr <<
                  "Usage: CallCentre <address> <port> <threads>\n" <<
                  "Example:\n" <<
                  "    CallCentre 0.0.0.0 8080 1\n";
        return EXIT_FAILURE;
    }

    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    net::io_context ioc{};

    // read config file
    const std::string kConfigFileName = "configCallCentre.json";
    call_c::ReadConfig jsonConf(kConfigFileName);

    call_c::Log::Init(jsonConf.log_server_level, jsonConf.log_operators_level);
    call_c::RandGen::Init(jsonConf.rand_gen_erl_shape, jsonConf.rand_gen_erl_scale, jsonConf.timeout_in_queue_min,
                          jsonConf.timeout_in_queue_max);

    call_c::Operators operators{ioc, jsonConf.count_operators, jsonConf.incoming_queue_size};
    call_c::Session::setIncomingCallsQueue(&operators.getIncCalls());
    // Create and launch a listening port
    std::make_shared<call_c::Listener>(
            ioc,
            tcp::endpoint{address, port}
            )->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&](beast::error_code const &, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all the sockets in it.
                ioc.stop();
            });

    // запускаю операторов
    std::thread thr_for_operators{
            [&operators]
            {
                operators.run();
            }
    };

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> threads_vector;
    threads_vector.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        threads_vector.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
    ioc.run();

    for (auto &t: threads_vector)
        t.join();

    operators.stop();
    thr_for_operators.join();

    return EXIT_SUCCESS;
}

// TODO: еще раз перечитать задание, почистить инклюды(using namespace), посмотреть pch.h, написать тесты,
// TODO: написать readme и добавить комментов