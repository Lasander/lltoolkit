#include "../../common/unittest/LogHelpers.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <thread>

#include "../ConcreteQueue.hpp"

using namespace testing;

namespace Data {
namespace {

} // anonymous namespace

TEST(ConcreteQueue, BasicIntQueue)
{
    ConcreteQueue<int> queue;
    EXPECT_TRUE(queue.isEmpty());

    queue.enqueue(42);
    EXPECT_FALSE(queue.isEmpty());

    queue.enqueue(33);
    EXPECT_EQ(42, queue.dequeue());
    queue.enqueue(99);
    EXPECT_EQ(33, queue.dequeue());
    EXPECT_EQ(99, queue.dequeue());
    EXPECT_TRUE(queue.isEmpty());
}

TEST(ConcreteQueue, BasicIntQueueMultipleThreads)
{
    struct Context
    {
        static void produce(ConcreteQueue<int>& queue, size_t count)
        {
            int lastSent = 0;
            for (size_t c = 0; c < count; ++c)
            {
                queue.enqueue(++lastSent);
            }
        }

        static void consume(ConcreteQueue<int>& queue, size_t count)
        {
            int lastReceived = 0;
            for (size_t c = 0; c < count; ++c)
            {
                ASSERT_EQ(++lastReceived, queue.dequeue());
            }
        }
    };

    ConcreteQueue<int> queue;

    std::thread consumer(&Context::consume, std::ref(queue), 10000);
    std::thread producer(&Context::produce, std::ref(queue), 10000);

    consumer.join();
    producer.join();
}

} // Data
