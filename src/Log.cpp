#include "Log.h"

namespace call_c
{
    std::shared_ptr<spdlog::logger> Log::cdrLog;
    std::shared_ptr<spdlog::logger> Log::serverLog;
    std::shared_ptr<spdlog::logger> Log::operatorsLog;

    std::map<std::string, spdlog::level::level_enum> Log::loggingLevels
            {
                    {"trace", spdlog::level::trace},
                    {"debug", spdlog::level::debug},
                    {"info", spdlog::level::info},
                    {"warn", spdlog::level::warn},
                    {"err", spdlog::level::err}
            };

    void Log::Init(const std::string& levelServer, const std::string& levelOperator)
    {
        cdrLog = spdlog::basic_logger_mt("CDR", "./logs/cdr.txt");
        cdrLog->set_level(spdlog::level::info);
        cdrLog->flush_on(spdlog::level::trace);


        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        serverLog = std::make_shared<spdlog::logger>("SERVER", sink);
        serverLog->set_level(loggingLevels[levelServer]);
        serverLog->flush_on(spdlog::level::trace);
        spdlog::register_logger(serverLog);

        operatorsLog = std::make_shared<spdlog::logger>("OPERATORS", sink);
        operatorsLog->set_level(loggingLevels[levelOperator]);
        operatorsLog->flush_on(spdlog::level::trace);
        spdlog::register_logger(operatorsLog);
    }

}

