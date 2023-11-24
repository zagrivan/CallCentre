#include "Log.h"

namespace call_c
{
    std::shared_ptr<spdlog::logger> Log::cdrLog;
    std::shared_ptr<spdlog::logger> Log::serverLog;
    std::shared_ptr<spdlog::logger> Log::operatorsLog;

    void Log::Init()
    {
        cdrLog = spdlog::basic_logger_mt("CDR", "./logs/cdr.txt");
        cdrLog->set_level(spdlog::level::trace);
        cdrLog->flush_on(spdlog::level::trace);


        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        serverLog = std::make_shared<spdlog::logger>("SERVER", sink);
        serverLog->set_level(spdlog::level::err);
        serverLog->flush_on(spdlog::level::trace);
        spdlog::register_logger(serverLog);

        operatorsLog = std::make_shared<spdlog::logger>("OPERATORS", sink);
        operatorsLog->set_level(spdlog::level::info);
        operatorsLog->flush_on(spdlog::level::trace);
        spdlog::register_logger(operatorsLog);
    }

}

