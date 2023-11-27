#ifndef CALLCENTRE_OPERATORSALTER_H
#define CALLCENTRE_OPERATORSALTER_H

/**
 * есть n операторов
 * операторы управляют очередью, могут удалять из нее звонок, если готовы принять его
 * оператор занимается на случайное время в диапозоне tmin - tmax
 * есть несколько событий:
 * появился первый запрос в очереди, отдаем его первому свободному оператору, но все операторы могут быть заняты
 * время жизни запроса в очереди истекло, нужно его удалить, после того как удалили этот запрос, проверяем следующий
 * все операторы заняты, но один освободился, если в очереди есть запросы, отдаем запрос освободившемуся оператору
 * оператор освободился, но очередь пуста
 *
 * можно поставить таймер на каждого оператора, а можно иметь один таймер,
 * который будет качественно подхватывать ближайшее завершение разговора
 *
 * как вариант, можно иметь два таймера, один будет обрабатывать incCalls, т.е. вытаскивать или удалять из очереди запрос,
 * если его время истекло,
 * второй будет ждать ближайшее завершение разговора
 *
 * либо использовать подход с cond_var, либо пытаться начать чтение из очереди, используя net::dispatch(strand, handler)
 * напрямую из сервера
 *
 * нужно не забыть запихнуть add_calls и set_timer в strand
 *
 */

#include <atomic>

#include "includes.h"

#include "handle_req_resp.h"
#include "Log.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace call_c
{

    using time_point = std::chrono::time_point<std::chrono::system_clock>;

    class OperatorsAlter
    {

    public:
        OperatorsAlter(net::io_context &ioc, int countOp, size_t incCallsSize, int Rmin, int Rmax, int callDuration)
                : ioc_(ioc), strand_(net::make_strand(ioc)), operators_(countOp),
                  count_(countOp), Rmin_(Rmin), Rmax_(Rmax), incCalls_(incCallsSize),
                  timerIncCall_(ioc), timerOperator_(ioc, std::chrono::days(100)),
                  freeOps_(countOp),
                  callDuration_(callDuration)
        {
            for (int i = 0; i != countOp; ++i)
            {
                freeOps_[i] = i;
            }
            cntFree_ = countOp;
            isTimerOperatorWorking_ = false;
        }


        tsqueue<std::shared_ptr<Call>> &getIncCalls()
        {
            return incCalls_;
        }

        void run()
        {
            C_OPERATORS_INFO("OPERATORS ARE RUNNING");
            while (1)
            {
                update();
            }
        }

        void stop()
        {

        }

    private:

        void update()
        {
            // ждем, пока появятся звонки в очереди
            incCalls_.wait();

            C_OPERATORS_DEBUG("INCQUEUE.wait() is awake, there are calls, in queue {} calls", incCalls_.count());

            if (cntFree_)  // можно обращаться к atomic bool, спрашивать есть ли свободные операторы, а в strand обращаться к freeOps_
            {
                C_OPERATORS_TRACE("there are {} free operators", cntFree_);
                --cntFree_;
                auto call = incCalls_.pop_front();
                net::dispatch(strand_, boost::bind(&OperatorsAlter::add_call, this, call));
            } else
            {
                C_OPERATORS_TRACE("there are no free operators, cntFree_ = {}", cntFree_);
                set_call_timer();
                // можно попробовать ждать здесь, пока не удалится крайний звонок в очереди
                C_OPERATORS_DEBUG("waiting for a free operator, in queue {} calls, cntFree_ = {}", incCalls_.count(),
                                  cntFree_);
                wait_free_op();
                // отменяем таймер и все асинх операции связанные с ним
                auto cancelled = timerIncCall_.cancel();
                C_OPERATORS_DEBUG("there is a {} free operator, {} async operations cancelled, timerIncCall_.cancel()",
                                  cntFree_, cancelled);
            }
        }

        void add_call(std::shared_ptr<Call> call)
        {
            auto OpID = get_free_operator();
            call->operatorID = OpID;
            call->dt_resp = std::chrono::system_clock::now();
            call->dt_completion = call->dt_resp + callDuration_;
            operators_[OpID] = call;
            nearestCallsEnd_.emplace(call->dt_completion, OpID);

            C_OPERATORS_INFO("CallID:{} was given to the OpID:{}, end of the call at {}",
                             call->callID, OpID, tp_to_strHMS(call->dt_completion));

            // если таймер сработает позже, чем конец нового звонка, или не запущен, то ставим таймер
            if (!isTimerOperatorWorking_ || timerOperator_.expiry() > call->dt_completion)
            {
                C_OPERATORS_DEBUG("timerOperator is {} working", (isTimerOperatorWorking_ ? "already" : "not"));
                isTimerOperatorWorking_ = true;
                timerOperator_.expires_at(nearestCallsEnd_.top().first);
                timerOperator_.async_wait(
                        boost::asio::bind_executor(
                                strand_,
                                boost::bind(&OperatorsAlter::on_end_call, this, net::placeholders::error,
                                            nearestCallsEnd_.top().second)));
                C_OPERATORS_DEBUG("timerOperator expires at {}", tp_to_strHMS(timerOperator_.expiry()));
            }
        }

        void set_call_timer()
        {
            // если свободных операторов нет, то ставим таймер и ждем,
            // пока сработает таймер, или освободится оператор
            auto call = incCalls_.front();
            call->dt_expiry = call->dt_req + Rmin_;
            auto cancelled = timerIncCall_.expires_at(call->dt_expiry);
            C_OPERATORS_TRACE("timerIncCall_.expires_at({}), cancelled = {}, CallID:{}", tp_to_strHMS(call->dt_expiry),
                              cancelled, call->callID);

            timerIncCall_.async_wait(
                    boost::asio::bind_executor(
                            strand_,
                            boost::bind(&OperatorsAlter::on_queue_expiry, this, net::placeholders::error, call->callID)));
        }

        void on_end_call(const boost::system::error_code &ec, int expectedOpID)
        {
            if (ec == net::error::operation_aborted)
            {
                C_OPERATORS_DEBUG("on_end_call: timerOperator expiry is changed, OpID:{}", expectedOpID);
                return;
            } else if (ec)
            {
                C_OPERATORS_ERROR("on_end_call: {}", ec.message());
                return;
            }

            auto OpID = nearestCallsEnd_.top().second;
            assert(OpID == expectedOpID && "expectedOpID != OpID from the heap");

            nearestCallsEnd_.pop();

//            auto call = operators_[OpID]; // TODO move
//            operators_[OpID] = nullptr;
            auto call = std::move(operators_[OpID]);

            C_OPERATORS_INFO("Call completed, CallID:{}, OpID:{}", call->callID, OpID);

            release_operator(OpID); // TODO где то здесь ошибка

            // заново устанавливаем таймер, если еще есть звонки в куче
            if (!nearestCallsEnd_.empty())
            {
                timerOperator_.expires_at(nearestCallsEnd_.top().first);
                timerOperator_.async_wait(
                        boost::asio::bind_executor(
                                strand_,
                                boost::bind(&OperatorsAlter::on_end_call, this, net::placeholders::error,
                                            nearestCallsEnd_.top().second)));
                C_OPERATORS_DEBUG("timerOperator expires at {}, from on_end_call()", tp_to_strHMS(timerOperator_.expiry()));
            } else
            {
                isTimerOperatorWorking_ = false; // timer is not working
                C_OPERATORS_DEBUG("timerOperator is not working, from on_end_call()");
            }

            // поместить оператора в свободных, залогировать вызов
        }

        void on_queue_expiry(const boost::system::error_code &ec, uint32_t expectedCallID)
        {
            // срабатывает, когда звонок в очереди ждет слишком долго,
            // время call->dt_expiry уже прошло
            if (ec == net::error::operation_aborted)
            {
                C_OPERATORS_DEBUG(
                        "on_queue_expiry: async operation is cancelled. The call was picked up by a free operator");
            } else if (ec)
            {
                C_OPERATORS_WARN("on_queue_expiry: {}", ec.message());
            } else
            {
                try
                {
                    if (incCalls_.front()->callID != expectedCallID)
                    {
                        C_OPERATORS_DEBUG("on_queue_expiry: expectedCallID:{} != CallID", expectedCallID);
                        return;
                    }
                    auto call = incCalls_.pop_front();

                    assert(call->callID == expectedCallID && "the incorrect call was removed from the queue");
                    C_OPERATORS_INFO("CallID:{} removed from the queue due to expiration date",
                                      call->callID);
                }
                catch (std::exception &e)
                {
                    C_OPERATORS_WARN("on_queue_expiry: {}", e.what());
                    return;
                }

                if (!incCalls_.empty() && !cntFree_)
                {
                    // ставим таймер на удаление запроса из очереди снова
                    set_call_timer();
                }
            }
        }

        void wait_free_op()
        {
            while (cntFree_ == 0)
            {
                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.wait(ul);
            }
        }

        int get_free_operator()
        {
            if (freeOps_.empty())
                throw std::out_of_range("freeOps is empty");
            auto OpID = freeOps_.front();
            freeOps_.pop_front();
//            --cntFree_;
            return OpID;
        }

        void release_operator(int OpID)
        {
            freeOps_.push_back(OpID);
            ++cntFree_;
            // будим update()
            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();
        }


    private:
        int count_;

        // time in queue
        std::chrono::seconds Rmin_;
        std::chrono::seconds Rmax_;

        // call duration
        std::chrono::seconds callDuration_;

        std::priority_queue<std::pair<time_point, int>, std::vector<std::pair<time_point, int>>, std::greater<>> nearestCallsEnd_{};

        net::io_context &ioc_;
        net::strand<net::io_context::executor_type> strand_;

        net::system_timer timerIncCall_;
        tsqueue<std::shared_ptr<Call>> incCalls_;

        std::atomic<bool> isTimerOperatorWorking_;
        net::system_timer timerOperator_;
        std::vector<std::shared_ptr<Call>> operators_;

        std::deque<int> freeOps_;
        std::atomic<int> cntFree_;

        std::condition_variable cvBlocking;
        std::mutex muxBlocking;
    };

}

#endif //CALLCENTRE_OPERATORSALTER_H
