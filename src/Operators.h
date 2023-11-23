#ifndef CALLCENTRE_OPERATORS_H
#define CALLCENTRE_OPERATORS_H

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


namespace call_c {

    using time_point = std::chrono::time_point<std::chrono::system_clock>;

    class Operators {

    public:
        Operators(net::io_context &ioc, int countOp, size_t incCallsSize, int Rmin, int Rmax, int callDuration)
                : ioc_(ioc), strand_(net::make_strand(ioc)), operators_(countOp),
                  count_(countOp), Rmin_(Rmin), Rmax_(Rmax), incCalls_(incCallsSize),
                  timerIncCall_(ioc), timerOperator_(ioc, std::chrono::days(100)),
                  freeOps_(countOp),
                  callDuration_(callDuration) {
            for (int i = 0; i != countOp; ++i) {
                freeOps_[i] = i;
            }
            cntFree_ = countOp;
            isTimerOperatorWorking_ = false;
        }


        tsqueue<Call> &getIncCalls() {
            return incCalls_;
        }

        void run() {
            C_OPERATORS_INFO("OPERATORS ARE RUNNING");
            while (1) {
                update();
            }
        }

        void stop() {

        }

    private:

        void update() {
            // здесь придется проверять, есть ли установленный таймер на крайний звонок в очереди
            // если таймер уже установлен то просто ждем его выполнения, если таймер не установлен, то обрабатываем крайний
            // звонок в очереди, либо закидываем на свободного оператора, либо устанавливаем таймер на вызов

            incCalls_.wait();

            C_OPERATORS_DEBUG("INCQUEUE.wait() is awake, there are calls, in queue {} calls", incCalls_.count());

            if (cntFree_)  // можно обращаться к atomic bool, спрашивать есть ли свободные операторы, а в strand обращаться к freeOps_
            {
                C_OPERATORS_TRACE("there are {} free operators", cntFree_);
                auto call = incCalls_.pop_front();
                auto OpID = get_free_operator();
                net::dispatch(strand_, boost::bind(&Operators::add_call, this, call, OpID));
            } else {
                C_OPERATORS_TRACE("there are no free operators, cntFree_ = {}", cntFree_);
                set_call_timer();
                // можно попробовать ждать здесь, пока не удалится крайний звонок в очереди
                C_OPERATORS_DEBUG("waiting for a free operator, in queue {} calls, cntFree_ = {}", incCalls_.count(), cntFree_);
                wait_free_op();
                // отменяем таймер и все асинх операции связанные с ним
                auto cancelled = timerIncCall_.cancel();
                C_OPERATORS_DEBUG("there is a {} free operator, {} async operations cancelled, timerIncCall_.cancel()", cntFree_, cancelled);
            }
        }

        void add_call(Call call, int OpID) {
            // если вдруг единственный звонок в очереди удалит on_queue_expiry, после проверки
//            if (incCalls_.empty()) {
//                C_OPERATORS_WARN("IN add_call(), BUT THERE ARE NO CALLS");
//                return;
//            }
//
//            auto call = incCalls_.pop_front();

            call.operatorID = OpID;
            call.dt_resp = std::chrono::system_clock::now();
            call.dt_completion = call.dt_resp + callDuration_;
            operators_[OpID] = call;
            nearestCallsEnd_.emplace(call.dt_completion, OpID);

            C_OPERATORS_INFO("CallID:{} was given to the OpID:{}, end of the call at {}",
                             call.callID, OpID, tp_to_strHMS(call.dt_completion));

            const auto currExpiry = timerOperator_.expiry();
            const auto now = std::chrono::system_clock::now(); // maybe unused
            // если таймер сработает позже, чем конец нового звонка, то меняем таймер
            if (!isTimerOperatorWorking_ || currExpiry > call.dt_completion) {
                C_OPERATORS_DEBUG("timerOperator is {} working", (isTimerOperatorWorking_ ? "already" : "not"));
                //TODO сделать функцию, которая переназначает таймер
                isTimerOperatorWorking_ = true;
                // число отмененных операций
                auto cancelled = timerOperator_.expires_at(nearestCallsEnd_.top().first);

                if (cancelled)
                    std::cerr << "add_call: number of cancelled operations " << cancelled << '\n'
                              << "\texpiry at: " << tp_to_strHMS(currExpiry)
                              << '\n' // если currExpiry < now(), то таймер уже сработал
                              << "\tnow: " << tp_to_strHMS(now) << '\n'
                              << std::flush; // но при этом откуда то взялись отмененные операции!!!

                timerOperator_.async_wait(
                        boost::asio::bind_executor(
                                strand_,
                                boost::bind(&Operators::on_end_call, this, net::placeholders::error,
                                            OpID))); //TODO boost::bind!!!
            }

            // ставим таймер и выходим из функции
        }

        void set_call_timer() {
            // если свободных операторов нет, то ставим таймер и ждем,
            // пока сработает таймер, или освободится оператор
            auto call = incCalls_.front();
            call.dt_expiry = call.dt_req + Rmin_;
            auto cancelled = timerIncCall_.expires_at(call.dt_expiry);
            C_OPERATORS_TRACE("timerIncCall_.expires_at({}), cancelled = {}", tp_to_strHMS(call.dt_expiry), cancelled);

            timerIncCall_.async_wait(
                    boost::asio::bind_executor(
                            strand_,
                            boost::bind(&Operators::on_queue_expiry, this, net::placeholders::error, call.callID)));
        }

        void on_end_call(const boost::system::error_code &ec, int expectedOpID) {
            if (ec == net::error::operation_aborted) {
                std::cout << "on_end_call: expiry is changed\tOpID: " << expectedOpID << '\n';
                return;
            } else if (ec) {
                fail(ec, "on_end_call");
                return;
            }

            auto OpID = nearestCallsEnd_.top().second;
            nearestCallsEnd_.pop();

            assert(OpID == expectedOpID && "expectedOpID != OpID from the heap");

            auto call = operators_[OpID];
//        operators_[OpID] = nullptr;

            std::cout << "on_end_call: end call\tOpID: " << OpID << '\n';
            std::cout << "call " << call.CgPN << "is completed\n";

            freeOps_.push_back(OpID);
            ++cntFree_;

            {
                // будим update()
                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.notify_one();
            }

            // заново устанавливаем таймер, если еще есть звонки в куче
            if (!nearestCallsEnd_.empty()) {
                auto newExpiry = nearestCallsEnd_.top().first;
                timerOperator_.expires_at(newExpiry);
                timerOperator_.async_wait(
                        boost::asio::bind_executor(
                                strand_,
                                boost::bind(&Operators::on_end_call, this, net::placeholders::error,
                                            nearestCallsEnd_.top().second))); //TODO boost::bind!!!
            } else {
                isTimerOperatorWorking_ = false; // timer is not working
            }

            // поместить оператора в свободных, залогировать вызов
        }

        void on_queue_expiry(const boost::system::error_code &ec, uint32_t callID) {
            // срабатывает, когда звонок в очереди ждет слишком долго,
            // время call->dt_expiry уже прошло
            if (ec == boost::asio::error::operation_aborted) {
                std::cerr
                        << "on_queue_expiry: async operation is cancelled. The call was picked up by a free operator\n";
            } else if (ec) {
                std::cerr << "on_queue_expiry: unknown error\n";
            } else {
                // отправляем в логгер, что звонок expiry
                if (incCalls_.empty() || incCalls_.front().callID != callID)
                    return;
                // TODO подумать над проблемой, когда звонков в очереди уже нет, но функция была отправлена в движок asio,
                // TODO и попыталась удалить из очереди
                auto call = incCalls_.pop_front();
                std::cout << "on_queue_expiry: callID_" << call.callID
                          << " removed from the queue due to expiration date\n";

                if (!incCalls_.empty() && !cntFree_) {
                    // ставим таймер на удаление запроса из очереди снова
                    set_call_timer();
                }
            }
        }

        void wait_free_op() {
            while (cntFree_ == 0) {
                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.wait(ul);
            }
        }

        int get_free_operator()
        {
            auto OpID = freeOps_.front();
            freeOps_.pop_front();
            --cntFree_;
            return OpID;
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
        tsqueue<Call> incCalls_;

        std::atomic<bool> isTimerOperatorWorking_;
        net::system_timer timerOperator_;
        std::vector<Call> operators_;

        std::deque<int> freeOps_;
        std::atomic<int> cntFree_;

        std::condition_variable cvBlocking;
        std::mutex muxBlocking;
    };

}

#endif //CALLCENTRE_OPERATORS_H
