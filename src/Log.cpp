#include "Log.h"
#include "handle_req_resp.h"

namespace call_c
{
    std::shared_ptr<spdlog::logger> Log::cdrLog;
    std::shared_ptr<spdlog::logger> Log::serverLog;
    std::shared_ptr<spdlog::logger> Log::operatorsLog;

    std::map<std::string, spdlog::level::level_enum> Log::loggingLevels
            {
                    {"trace", spdlog::level::trace},
                    {"debug", spdlog::level::debug},
                    {"info",  spdlog::level::info},
                    {"warn",  spdlog::level::warn},
                    {"err",   spdlog::level::err}
            };

    void Log::Init(const std::string &levelServer, const std::string &levelOperator)
    {
        cdrLog = spdlog::basic_logger_mt("CDR", "./logs/cdr.txt");
        cdrLog->set_pattern("%v");
        cdrLog->set_level(spdlog::level::info);
        cdrLog->flush_on(spdlog::level::info);

        cdrLog->info(
                "[{}] START OF RECORDING: DT_req; CallID; CgPn; DT_completion; status; DT_operator_resp; Operator_ID; call_duration_secunds;",
                tp_to_strYMD(std::chrono::system_clock::now()) + " " + tp_to_strHMS(std::chrono::system_clock::now()));

        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // [2014-10-31 23:46:59.678] [my_loggername] [info] Some message
        sink->set_pattern("[%Y-%m-%d %T.%e]%^[%-9n] [%-8l]%$ %v");

        serverLog = std::make_shared<spdlog::logger>("SERVER", sink);
        serverLog->set_level(loggingLevels[levelServer]);
        serverLog->flush_on(spdlog::level::trace);
        spdlog::register_logger(serverLog);

        operatorsLog = std::make_shared<spdlog::logger>("OPERATORS", sink);
        operatorsLog->set_level(loggingLevels[levelOperator]);
        operatorsLog->flush_on(spdlog::level::trace);
        spdlog::register_logger(operatorsLog);
    }

    void Log::WriteCDR(const std::shared_ptr<Call> &call)
    {
        auto dt_req = tp_to_strYMD(call->dt_req) + " " + tp_to_strHMS(call->dt_req);
        auto dt_completion = tp_to_strYMD(call->dt_completion) + " " + tp_to_strHMS(call->dt_completion);

        std::string dt_operator_resp = "-";
        std::string OpID = "-";
        std::string call_duration = "-";
        std::string status;

        switch (call->status)
        {
            case RespStatus::OK:
                dt_operator_resp = tp_to_strHMS(call->dt_resp);
                OpID = std::to_string(call->operatorID);
                call_duration = std::to_string(
                        duration_cast<std::chrono::seconds>(call->dt_completion - call->dt_resp).count());
                status = "OK";
                break;
            case RespStatus::OVERLOAD:
                status = "OVERLOAD";
                break;
            case RespStatus::TIMEOUT:
                status = "TIMEOUT";
                break;
            case RespStatus::ALREADY_IN_QUEUE:
                status = "ALREADY_IN_QUEUE";
                break;
        }

        cdrLog->info("{} ; {} ; {} ; {} ; {} ; {} ; {} ; {};", dt_req, call->callID, call->CgPN, dt_completion, status,
                     dt_operator_resp, OpID, call_duration);
    }

}

