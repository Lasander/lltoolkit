#ifndef DATA_QUEUE_HPP_
#define DATA_QUEUE_HPP_

#include "../common/Semaphore.hpp"
#include <deque>

namespace Data {

template <typename T>
class Queue
{
public:
    Queue() :
        queue_(),
        dequedElement_(nullptr),
        semaphore_(0)
    {
    }

    virtual ~Queue() {}

    void enqueue(const T& element)
    {
        {
            std::lock_guard<std::mutex> lock{mutex_};
            queue_.push_back(element);
        }
        semaphore_.notify();
    }

    bool isEmpty() const
    {
        std::lock_guard<std::mutex> lock{mutex_};
        if (dequedElement_)
        {
            return queue_.size() < 2;
        }
        return queue_.empty();
    }

    const T& dequeue()
    {
        if (dequedElement_)
        {
            // Release previously dequeued element
            std::lock_guard<std::mutex> lock{mutex_};
            queue_.pop_front();
            dequedElement_ = nullptr;
        }

        while (queue_.empty())
        {
            semaphore_.wait();
        }

        std::lock_guard<std::mutex> lock{mutex_};
        dequedElement_ = &queue_.front();
        return *dequedElement_;
    }

private:
    /** Non-copyable */
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    std::deque<T> queue_;
    const T* dequedElement_;
    Common::Semaphore semaphore_;
    mutable std::mutex mutex_;
};

}

#endif
