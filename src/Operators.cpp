#include "Operators.h"
#include "Log.h"

// TODO: возможно tsqueu больше не нужна
namespace call_c
{

    Operators::Operators(net::io_context &ioc, int countOp, size_t incCallsSize)
            : ioc_(ioc), incCalls_(incCallsSize, ioc)
    {
        operators_.reserve(countOp);
        for (int i = 0; i != countOp; ++i)
        {
            freeOps_.push_back(i);
            operators_.emplace_back(ioc);
        }
        LOG_OPERATORS_INFO("Number of operators = {}, the queue size = {}", countOp, incCallsSize);
    }

    void Operators::run()
    {
        LOG_OPERATORS_INFO("OPERATORS ARE RUNNING");
        while (true)
        {
            // ждем, пока появятся операторы и звонки в очереди
            freeOps_.wait();
            incCalls_.wait();

            if (!continue_loop_)
                return;

            add_call();
        }
    }

    void Operators::stop()
    {
        continue_loop_ = false;
        freeOps_.push_back(0);
        incCalls_.awake();
    }

    void Operators::add_call()
    {
        std::shared_ptr<Call> call;
        try
        {
            call = incCalls_.pop();
        }
        catch (const std::out_of_range& e)
        {
            LOG_OPERATORS_ERROR("Operators::add_call(): {}", e.what());
            return;
        }

        auto OpId = freeOps_.pop_front();
        call->SetOperatorResponseData(OpId);

        operators_[OpId].expires_after(call->call_duration);
        operators_[OpId].async_wait(boost::bind(&Operators::on_end_call,
                                                this, net::placeholders::error,call));

        LOG_OPERATORS_INFO("CallID:{} CgPn:{} OpId:{} Время завершения:{} РАЗГОВОР НАЧАЛСЯ",
                           call->callID, call->CgPn, OpId, tp_to_strHMS(call->dt_completion));
    }

    void Operators::on_end_call(const boost::system::error_code &ec, std::shared_ptr<Call> call)
    {
        if (ec)
        {
            LOG_OPERATORS_ERROR("on_end_call: {}", ec.message());
            return;
        }

        freeOps_.push_back(call->operatorID);
        call->status = Call::RespStatus::OK;
        Log::WriteCDR(call);

        LOG_OPERATORS_INFO("CallID:{} CgPn:{} OpId:{} ЗВОНОК ЗАВЕРШЕН", call->callID, call->CgPn, call->operatorID);
    }

}