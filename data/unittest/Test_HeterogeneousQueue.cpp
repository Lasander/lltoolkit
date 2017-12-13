#include "../../common/unittest/LogHelpers.hpp"
#include "../HeterogeneousQueue.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <string>
#include <thread>

using namespace testing;

namespace Data {

TEST(HeterogeneousQueue, IntQueue)
{
    HeterogeneousQueue<int> queue(256);
    EXPECT_TRUE(queue.isEmpty());

    queue.enqueue(42);
    EXPECT_FALSE(queue.isEmpty());

    queue.enqueue(33);
    EXPECT_EQ(42, queue.dequeue());

    queue.enqueue(99);
    EXPECT_EQ(33, queue.dequeue());
    EXPECT_EQ(99, queue.dequeue());

    queue.enqueue(5);
    EXPECT_EQ(5, queue.dequeue());
    EXPECT_TRUE(queue.isEmpty());
}

TEST(HeterogeneousQueue, IntQueueMultipleThreads)
{
    using Queue = HeterogeneousQueue<int>;

    struct Context
    {
        static void produce(Queue& queue, size_t count)
        {
            int lastSent = 0;
            for (size_t c = 0; c < count; ++c)
            {
                queue.enqueue(++lastSent);
            }
        }

        static void consume(Queue& queue, size_t count)
        {
            int lastReceived = 0;
            for (size_t c = 0; c < count; ++c)
            {
                ASSERT_EQ(++lastReceived, queue.dequeue());
            }
        }
    };

    Queue queue(128);

    std::thread consumer(&Context::consume, std::ref(queue), 10000);
    std::thread producer(&Context::produce, std::ref(queue), 10000);

    consumer.join();
    producer.join();
}

namespace {

enum ElementId
{
    EMPTY_ELEMENT,
    INT_ELEMENT,
    DOUBLE_ELEMENT,
    STRING_ELEMENT
};

static std::atomic<int> elementCounterS(0);

class ElementIf
{
public:
    ElementIf() { ++elementCounterS; }
    virtual ElementId getId() const = 0;
    virtual ~ElementIf() { --elementCounterS; }
};

class EmptyElement : public ElementIf
{
public:
    static const ElementId ID_T = EMPTY_ELEMENT;
    virtual ElementId getId() const { return ID_T; }

    EmptyElement() = default;
    EmptyElement(EmptyElement&&) {}
    EmptyElement(const EmptyElement&) {}
};

template <ElementId ID, typename DataType>
class Element : public ElementIf
{
public:
    static const ElementId ID_T = ID;

    Element() : data_{} {}
    Element(const DataType& data) : data_{data} {}
    Element(DataType&& data) : data_{std::move(data)} {}
    Element(const Element& other) : data_{other.data_} {}
    Element(Element&& other) : data_{std::move(other.data_)} {}
    ~Element() = default;

    Element& operator=(Element&& other)
    {
        if (&other != this)
        {
            data_ = std::move(other.data_);
        }
        return *this;
    }
    virtual ElementId getId() const { return ID_T; }
    const DataType& data() const { return data_; }

private:
    DataType data_;
};

using IntElement = Element<INT_ELEMENT, int>;
using DoubleElement = Element<DOUBLE_ELEMENT, double>;
using StringElement = Element<STRING_ELEMENT, std::string>;

template <typename T>
const T& element_cast(const ElementIf& element)
{
    assert(T::ID_T == element.getId());
    return static_cast<const T&>(element);
}

} // anonymous namespace

TEST(HeterogeneousQueue, ElementQueue)
{
    elementCounterS = 0;
    {
        HeterogeneousQueue<ElementIf> queue(256);
        EXPECT_TRUE(queue.isEmpty());

        for (int i = 0; i < 1000; ++i)
        {
            queue.enqueue(EmptyElement());
            EXPECT_FALSE(queue.isEmpty());

            queue.enqueue(DoubleElement(3.1415));
            EXPECT_EQ(EMPTY_ELEMENT, queue.dequeue().getId());

            queue.enqueue(StringElement("Brown fox jumps over the lazy dog and does this and that"));
            EXPECT_EQ(3.1415, (element_cast<DoubleElement>(queue.dequeue())).data());
            EXPECT_EQ(
                std::string("Brown fox jumps over the lazy dog and does this and that"),
                (element_cast<StringElement>(queue.dequeue())).data());
            EXPECT_TRUE(queue.isEmpty());
        }

        // Leave some elements to the queue when it's destructed
        queue.enqueue(EmptyElement());
        queue.enqueue(DoubleElement(3.1415));
        queue.enqueue(StringElement("Brown fox jumps over the lazy dog and does this and that"));
    }
    EXPECT_EQ(0, elementCounterS);
}

} // namespace Data
