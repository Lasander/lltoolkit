#include "../../Common/unittest/LogHelpers.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <thread>
#include "../Queue.hpp"

using namespace testing;

namespace Data {
namespace {

} // anonymous namespace

TEST(Queue, BasicIntQueue)
{
    Queue<int> queue;
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

TEST(Queue, BasicIntQueueMultipleThreads)
{
    struct Context
    {
        static void produce(Queue<int>& queue, size_t count)
        {
            int lastSent = 0;
            for (size_t c = 0; c < count; ++c)
            {
                queue.enqueue(++lastSent);
            }
        }

        static void consume(Queue<int>& queue, size_t count)
        {
            int lastReceived = 0;
            for (size_t c = 0; c < count; ++c)
            {
                ASSERT_EQ(++lastReceived, queue.dequeue());
            }
        }
    };

    Queue<int> queue;

    std::thread consumer(&Context::consume, std::ref(queue), 10000);
    std::thread producer(&Context::produce, std::ref(queue), 10000);

    consumer.join();
    producer.join();
}

} // Data
