#include "Operators.h"
#include "Log.h"
#include "handle_req_resp.h"
#include "RandGen.h"


namespace call_c
{

    Operators::Operators(net::io_context &ioc, int countOp, size_t incCallsSize)
            : ioc_(ioc), strand_(net::make_strand(ioc)), incCalls_(incCallsSize),
              timerIncCall_(ioc), freeOps_(countOp)
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
        while (1)
        {
            update();
        }
    }

    void Operators::update()
    {
        // ждем, пока появятся звонки в очереди
        incCalls_.wait();

        LOG_OPERATORS_DEBUG("INCQUEUE.wait() is awake, there are calls, in queue {} calls", incCalls_.count());

        if (!freeOps_.empty())
        {
            LOG_OPERATORS_TRACE("there are {} free operators", freeOps_.count());
            add_call();
        } else
        {
            LOG_OPERATORS_TRACE("there are no free operators, cntFree_ = {}", freeOps_.count());
            set_call_timer();
            // можно попробовать ждать здесь, пока не удалится крайний звонок в очереди
            LOG_OPERATORS_DEBUG("waiting for a free operator, in queue {} calls, cntFree_ = {}", incCalls_.count(),
                                freeOps_.count());
            freeOps_.wait();
            // отменяем таймер и все асинх операции связанные с ним
            auto cancelled = timerIncCall_.cancel();
            LOG_OPERATORS_DEBUG("there is a {} free operator, {} async operations cancelled, timerIncCall_.cancel()",
                                freeOps_.count(), cancelled);
        }
    }

    void Operators::add_call()
    {
        auto call = incCalls_.pop_front();
        auto OpID = freeOps_.pop_front();
        call->operatorID = OpID;
        call->dt_resp = std::chrono::system_clock::now();
        call->dt_completion = call->dt_resp + RandGen::getRandErlang();

        operators_[OpID].expires_at(call->dt_completion);
        operators_[OpID].async_wait(
                boost::asio::bind_executor(
                        strand_,
                        boost::bind(&Operators::on_end_call, this, net::placeholders::error,
                                    call)));

        LOG_OPERATORS_INFO("CallID:{} was given to the OpID:{}, end of the call at {}",
                           call->callID, OpID, tp_to_strHMS(call->dt_completion));
    }

    void Operators::set_call_timer()
    {
        // ставим таймер и ждем, пока сработает таймер, или освободится оператор
        auto call = incCalls_.front();
        call->dt_expiry = call->dt_req + RandGen::getExpirationTime();
        auto cancelled = timerIncCall_.expires_at(call->dt_expiry);
        LOG_OPERATORS_TRACE("timerIncCall_.expires_at({}), cancelled:{}, CallID:{}, req time:{}", tp_to_strHMS(call->dt_expiry),
                            cancelled, call->callID, tp_to_strHMS(call->dt_req));

        timerIncCall_.async_wait(
                boost::asio::bind_executor(
                        strand_,
                        boost::bind(&Operators::on_queue_expiry, this, net::placeholders::error, call->callID)));
    }

    void Operators::on_end_call(const boost::system::error_code &ec, std::shared_ptr<Call> call)
    {
        if (ec)
        {
            LOG_OPERATORS_ERROR("on_end_call: {}", ec.message());
            return;
        }

        freeOps_.push_back(call->operatorID);
        call->status = RespStatus::OK;
        Log::WriteCDR(call);

        LOG_OPERATORS_INFO("Call completed, CallID:{}, OpID:{}", call->callID, call->operatorID);
    }

    void Operators::on_queue_expiry(const boost::system::error_code &ec, uint32_t expectedCallID)
    {
        // срабатывает, когда звонок в очереди ждет слишком долго,
        // время call->dt_expiry уже прошло
        if (ec == net::error::operation_aborted)
        {
            LOG_OPERATORS_DEBUG(
                    "on_queue_expiry: async operation is cancelled. The call was picked up by a free operator");
        } else if (ec)
        {
            LOG_OPERATORS_WARN("on_queue_expiry: {}", ec.message());
        } else
        {
            try
            {
                if (incCalls_.front()->callID != expectedCallID)
                {
                    LOG_OPERATORS_DEBUG("on_queue_expiry: expectedCallID:{} != CallID", expectedCallID);
                    return;
                }

                auto call = incCalls_.pop_front();

                assert(call->callID == expectedCallID && "the incorrect call was removed from the queue");
                LOG_OPERATORS_INFO("CallID:{} removed from the queue due to expiration date",
                                   call->callID);

                call->status = RespStatus::TIMEOUT;
                call->dt_completion = std::chrono::system_clock::now();
                Log::WriteCDR(call);
            }
            catch (std::exception &e)
            {
                LOG_OPERATORS_WARN("on_queue_expiry: {}", e.what());
                return;
            }

            if (!incCalls_.empty() && freeOps_.empty())
            {
                // ставим таймер на удаление запроса из очереди снова
                set_call_timer();
            }
        }
    }


}