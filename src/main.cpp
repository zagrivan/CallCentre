#include "includes.h"

#include <boost/asio/signal_set.hpp>

#include "Listener.h"
#include "Log.h"
#include "Operators.h"
#include "RandGen.h"
#include "ReadConfig.h"


namespace net = boost::asio;            // from <boost/asio.hpp>


void Start(const net::ip::address& address, ushort port, int threads_num, const std::string& config_fn)
{
    // The io_context is required for all I/O
    net::io_context ioc{};

    // read config file
    call_c::ReadConfig json_data(config_fn);

    call_c::Log::Init(json_data.log_server_level, json_data.log_operators_level);
    call_c::RandGen::Init(json_data.rand_gen_erl_shape, json_data.rand_gen_erl_scale, json_data.timeout_in_queue_min,
                          json_data.timeout_in_queue_max);

    call_c::Operators operators{ioc, json_data.count_operators, json_data.incoming_queue_size};

    // Create and launch a listening port
    std::make_shared<call_c::Listener>(
            ioc,
            tcp::endpoint{address, port},
            operators.getIncCalls()
    )->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&](beast::error_code const &, int)
            {
                ioc.stop();
            });

    // запускаю операторов
    std::thread thr_for_operators{
            [&operators]
            {
                operators.Run();
            }
    };

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> threads_vector;
    threads_vector.reserve(threads_num - 2);
    for (auto i = threads_num - 2; i > 0; --i)
        threads_vector.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });

    ioc.run();

    for (auto &t: threads_vector)
        t.join();

    // останавливаю операторов, после того как все асинх операции отменены
    operators.Stop();
    thr_for_operators.join();
}


int main(int argc, char *argv[])
{
    // Check command line arguments.
    if (argc != 4 && argc != 5)
    {
        std::cerr <<
                  "Usage: CallCentre <address> <port> <threads> <config file>\n" <<
                  "Example:\n" <<
                  "    CallCentre 0.0.0.0 8080 2 config_call_centre.json\n";
        return EXIT_FAILURE;
    }

    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::stoi(argv[2]));
    // минимум два thread, под операторов и под io_context
    auto const threads = std::max<int>(2, std::stoi(argv[3]));

    // имя или путь до конфига
    const std::string config_fn = argc == 5 ? argv[4] : "config_call_centre.json";

    Start(address, port, threads, config_fn);

    return 0;
}
