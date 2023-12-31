#include "Operators.h"
#include "Log.h"


namespace call_c
{

    Operators::Operators(net::io_context &ioc, int count_op, size_t incCallsSize) : incoming_calls_(incCallsSize, ioc)
    {
        operators_.reserve(count_op);
        for (int i = 0; i != count_op; ++i)
        {
            free_operators_.push_back(i);
            operators_.emplace_back(ioc);
        }
        LOG_OPERATORS_INFO("Number of operators = {}, the queue size = {}", count_op, incCallsSize);
    }

    void Operators::Run()
    {
        LOG_OPERATORS_INFO("OPERATORS ARE RUNNING");
        while (true)
        {
            // ждем, пока появятся операторы и звонки в очереди
            // push в очередь пробудит
            free_operators_.wait();
            incoming_calls_.wait();

            if (!continue_loop_)
                return;

            AddCall();
        }
    }

    void Operators::Stop()
    {
        continue_loop_ = false;
        free_operators_.push_back(0);
        incoming_calls_.awake();
    }

    void Operators::AddCall()
    {
        std::shared_ptr<Call> call;
        try
        {
            call = incoming_calls_.pop();
        }
        catch (const std::out_of_range& e)
        {
            LOG_OPERATORS_ERROR("Operators::AddCall(): {}", e.what());
            return;
        }

        auto OpId = free_operators_.pop_front();
        call->SetOperatorResponseData(OpId);

        operators_[OpId].expires_after(call->call_duration);
        operators_[OpId].async_wait([this, call] (const boost::system::error_code &ec)
                                    {
                                        this->OnEndCall(ec, call);
                                    });

        LOG_OPERATORS_INFO("CallID:{} CgPn:{} OpId:{} START OF CALL", call->call_id, call->cg_pn, OpId);

    }

    void Operators::OnEndCall(const boost::system::error_code &ec, const std::shared_ptr<Call>& call)
    {
        if (ec)
        {
            LOG_OPERATORS_ERROR("OnEndCall: {}", ec.message());
            return;
        }
        LOG_OPERATORS_INFO("CallID:{} CgPn:{} OpId:{} CALL_IS_COMPLETED", call->call_id, call->cg_pn, call->operator_id);

        free_operators_.push_back(call->operator_id);

        call->SetCompleteData(Call::RespStatus::OK);

        Log::WriteCDR(call);
    }

}