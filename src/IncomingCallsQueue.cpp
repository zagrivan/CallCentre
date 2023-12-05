#include "IncomingCallsQueue.h"


namespace call_c
{
    IncomingCallsQueue::IncomingCallsQueue(size_t capacity, net::io_context &ioc) : capacity_(capacity)
    {
        timers_.reserve(capacity);
        for (int i = 0; i != capacity; ++i)
        {
            timers_.emplace_back(ioc); // initializing a timer
            free_timers_.push(i);      // timer index
        }
    }

    const std::shared_ptr<Call> &IncomingCallsQueue::front() {
        std::scoped_lock lock(mux_queue);
        if (incoming_calls_.empty())
            throw std::out_of_range("IncomingCallsQueue is empty, front() is forbidden");
        return incoming_calls_.front();
    }

    const std::shared_ptr<Call> &IncomingCallsQueue::back() {
        std::scoped_lock lock(mux_queue);
        if (incoming_calls_.empty())
            throw std::out_of_range("IncomingCallsQueue is empty, back() is forbidden");
        return incoming_calls_.back();
    }


    Call::RespStatus IncomingCallsQueue::push(const std::shared_ptr<Call> &call) {

        std::scoped_lock lock(mux_queue);
        // проверка на дублирование
        if (map_cg_pn_to_pos_.find(call->cg_pn) != map_cg_pn_to_pos_.end())
        {
            LOG_OPERATORS_INFO("CallID:{} CgPn:{} ALREADY_IN_QUEUE", call->call_id, call->cg_pn);
            return Call::RespStatus::ALREADY_IN_QUEUE;
        }

        if (incoming_calls_.size() >= capacity_)
        {
            LOG_OPERATORS_INFO("CallID:{} CgPn:{} QUEUE_OVERLOAD", call->call_id, call->cg_pn);
            return Call::RespStatus::OVERLOAD;
        }

        // индекс свободного таймера
        int timer_index = free_timers_.front();
        free_timers_.pop();
        // добавляем в саму очередь вызовов
        incoming_calls_.push_back(call);
        // добавляем в мапу, чтобы связать вызов и индекс таймера в векторе
        map_cg_pn_to_pos_.insert({call->cg_pn, timer_index});
        // ставлю таймер связанный с этим вызовом, при срабатывании он удалит его из очереди
        // если успеет сработать до того, как вызовется this->pop() оператором
        timers_[timer_index].expires_after(call->expiration_time);
        timers_[timer_index].async_wait(
                [this, CgPn = call->cg_pn, iter = --incoming_calls_.end()]
                        (const boost::system::error_code &ec)
                        {
                            this->OnCallReset(ec, CgPn, iter);
                        });

        LOG_OPERATORS_DEBUG("CallID:{} timer_index:{} IncomingCallsQueue::push", call->call_id, timer_index);

        std::unique_lock<std::mutex> ul(mux_blocking); // wake up wait function
        cv_blocking.notify_one();

        return Call::RespStatus::OK;
    }

    std::shared_ptr<Call> IncomingCallsQueue::pop() {
        std::scoped_lock lock(mux_queue);

        if (incoming_calls_.empty())
            throw std::out_of_range("IncomingCallsQueue is empty, pop_front() is forbidden");

        auto call = incoming_calls_.front();
        incoming_calls_.pop_front();

        int timer_index = map_cg_pn_to_pos_.at(call->cg_pn);
        map_cg_pn_to_pos_.erase(call->cg_pn);
        // отменяю асинхронную операцию на связанном таймере
        timers_[timer_index].cancel();

        free_timers_.push(timer_index);

        return call;
    }

    bool IncomingCallsQueue::empty() {
        std::scoped_lock lock(mux_queue);
        return incoming_calls_.empty();
    }

    size_t IncomingCallsQueue::size() {
        std::scoped_lock lock(mux_queue);
        return incoming_calls_.size();
    }

    void IncomingCallsQueue::clear() {
        std::scoped_lock lock(mux_queue);
        incoming_calls_.clear();
    }

    void IncomingCallsQueue::wait()
    {
        while (empty()) {
            std::unique_lock<std::mutex> ul(mux_blocking);
            cv_blocking.wait(ul);
        }
    }

    void IncomingCallsQueue::awake()
    {
        std::scoped_lock lock(mux_queue);
        incoming_calls_.emplace_back(nullptr);
        std::unique_lock<std::mutex> ul(mux_blocking); // wake up wait function
        cv_blocking.notify_one();
    }

    void IncomingCallsQueue::OnCallReset(const boost::system::error_code &ec, std::string cg_pn, ListIterator iter)
    {
        if (ec == net::error::operation_aborted) // при отмене таймера
        {
            return;
        }
        else if (ec)
        {
            LOG_OPERATORS_ERROR("IncomingCallsQueue::OnCallReset: {}", ec.message());
            return;
        }

        std::scoped_lock lock(mux_queue);

        // если вдруг этот completion handler уже был отправлен на обработку, но звонок забрали
        if (map_cg_pn_to_pos_.find(cg_pn) == map_cg_pn_to_pos_.end())
            return;

        int timer_index = map_cg_pn_to_pos_.at(cg_pn);
        map_cg_pn_to_pos_.erase(cg_pn);

        std::shared_ptr<Call> call{*iter};
        call->SetCompleteData(Call::RespStatus::TIMEOUT);

        // удалили элемент из очереди
        incoming_calls_.erase(iter);
        free_timers_.push(timer_index);

        LOG_OPERATORS_INFO("CallID:{} CgPn:{} TIMEOUT", call->call_id, cg_pn);
        Log::WriteCDR(call);
    }
}