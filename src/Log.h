#ifndef CALLCENTRE_LOG_FUNC_H
#define CALLCENTRE_LOG_FUNC_H

#include <iostream>
#include <map>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "Call.h"

namespace call_c
{
    class Log
    {
    public:
        static void Init(const std::string& levelServer = "trace", const std::string& levelOperator = "trace");

        static std::shared_ptr<spdlog::logger>& GetCDRLogger() { return cdrLog; }
        static std::shared_ptr<spdlog::logger>& GetServerLogger() { return serverLog; }
        static std::shared_ptr<spdlog::logger>& GetOperatorsLogger() { return operatorsLog; }

    private:
        static std::map<std::string, spdlog::level::level_enum> loggingLevels;

        static std::shared_ptr<spdlog::logger> cdrLog;
        static std::shared_ptr<spdlog::logger> serverLog;
        static std::shared_ptr<spdlog::logger> operatorsLog;
    };

}

// CDR log macros
#define C_CDR_TRACE(...) ::call_c::Log::GetCDRLogger()->trace(__VA_ARGS__)
//TODO дописать CDR


// server log macros
#define C_SERVER_TRACE(...)         ::call_c::Log::GetServerLogger()->trace(__VA_ARGS__)
#define C_SERVER_DEBUG(...)         ::call_c::Log::GetServerLogger()->debug(__VA_ARGS__)
#define C_SERVER_INFO(...)          ::call_c::Log::GetServerLogger()->info(__VA_ARGS__)
#define C_SERVER_WARN(...)          ::call_c::Log::GetServerLogger()->warn(__VA_ARGS__)
#define C_SERVER_ERROR(...)         ::call_c::Log::GetServerLogger()->error(__VA_ARGS__)
#define C_SERVER_CRITICAL(...)      ::call_c::Log::GetServerLogger()->critical(__VA_ARGS__)

// operators log macros
#define C_OPERATORS_TRACE(...)         ::call_c::Log::GetOperatorsLogger()->trace(__VA_ARGS__)
#define C_OPERATORS_DEBUG(...)         ::call_c::Log::GetOperatorsLogger()->debug(__VA_ARGS__)
#define C_OPERATORS_INFO(...)          ::call_c::Log::GetOperatorsLogger()->info(__VA_ARGS__)
#define C_OPERATORS_WARN(...)          ::call_c::Log::GetOperatorsLogger()->warn(__VA_ARGS__)
#define C_OPERATORS_ERROR(...)         ::call_c::Log::GetOperatorsLogger()->error(__VA_ARGS__)
#define C_OPERATORS_CRITICAL(...)      ::call_c::Log::GetOperatorsLogger()->critical(__VA_ARGS__)



#endif //CALLCENTRE_LOG_FUNC_H
