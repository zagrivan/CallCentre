#include <atomic>

#include <gtest/gtest.h>
#include <chrono>

#include "Log.h"
#include "RandGen.h"
#include "IncomingCallsQueue.h"
#include "Call.h"


using namespace call_c;


class IncomingCallsQueueTest : public testing::Test
{
protected:
    IncomingCallsQueueTest() : ioc_(), queue_(capacity_, ioc_), ioc_working_(true)
    {
        t_ = std::thread([&io = ioc_, &working = ioc_working_]{
            auto work = net::make_work_guard(io);
            io.run();
            working = false;
        });

    }

    ~IncomingCallsQueueTest() override
    {
        ioc_.stop();
        t_.join();
    }

protected:
    int capacity_ = 5;
    net::io_context ioc_;
    IncomingCallsQueue queue_;

    std::thread t_;

    std::atomic<bool> ioc_working_;

};


TEST_F(IncomingCallsQueueTest, IsCorrectInitialization)
{
    ASSERT_TRUE(ioc_working_.load());
    ASSERT_TRUE(queue_.empty());
    EXPECT_EQ(queue_.size(), 0);
}

TEST_F(IncomingCallsQueueTest, ExceptionTest)
{
    EXPECT_THROW(queue_.front(), std::out_of_range);
    EXPECT_THROW(queue_.back(), std::out_of_range);
    EXPECT_THROW(queue_.pop(), std::out_of_range);
}


TEST_F(IncomingCallsQueueTest, QueuePushAndPopIsWorks)
{
    for (int i = 0; i != capacity_; ++i)
    {
        queue_.push(std::make_shared<Call>(std::to_string(i + 100)));
    }
    ASSERT_EQ(queue_.size(), capacity_);
    for (int i = 0; i != capacity_; ++i)
    {
        auto call = queue_.pop();
        ASSERT_EQ(call->CgPn, std::to_string(i + 100));
    }
    ASSERT_EQ(queue_.size(), 0);
}

TEST_F(IncomingCallsQueueTest, QueuePushReturnCorrectEnum)
{
    for (int i = 0; i != capacity_; ++i)
    {
        EXPECT_EQ(queue_.push(std::make_shared<Call>(std::to_string(i + 100))), Call::RespStatus::OK);
    }
    EXPECT_EQ(queue_.push(std::make_shared<Call>(queue_.front()->CgPn)), Call::RespStatus::ALREADY_IN_QUEUE);
    EXPECT_EQ(queue_.push(std::make_shared<Call>(std::to_string(capacity_ + 100))), Call::RespStatus::OVERLOAD);
}

TEST_F(IncomingCallsQueueTest, DeletionFromQueueByTimeout)
{
    RandGen::setRandUniform(0.001, 0.002);
    for (int i = 0; i != capacity_; ++i)
    {
        queue_.push(std::make_shared<Call>(std::to_string(i + 100)));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(queue_.size(), 0);
    RandGen::setRandUniform(2, 4);
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    Log::Init("off", "off");
    RandGen::Init(10, 1, 2, 4);

    return RUN_ALL_TESTS();
}