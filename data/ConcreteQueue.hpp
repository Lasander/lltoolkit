#pragma once

#include "../common/Semaphore.hpp"
#include <deque>

namespace Data {

/**
 * Dynamic size queue of objects of type @p T.
 *
 * Meant for one reader and one (concurrent) writer.
 *
 * The objects are copied to the buffer itself so they need to be
 * copyable.
 */
template <typename T>
class ConcreteQueue
{
public:
    ConcreteQueue();
    virtual ~ConcreteQueue();

    /** Put (a copy of) a new @p element to the end of the queue. */
    void enqueue(const T& element);

    /** @return true if there are queue items. */
    bool isEmpty() const;

    /**
     * Fetch the next element from the queue. If there are no queued
     * elements (@see isEmpty), the call will block until an element
     * is inserted.
     */
    const T& dequeue();

private:
    // Non-copyable
    // TODO: Could be made copyable as all elements are there by value.
    //       However, not sure what would be the use case.
    ConcreteQueue(const ConcreteQueue&) = delete;
    ConcreteQueue& operator=(const ConcreteQueue&) = delete;

    std::deque<T> queue_;
    const T* dequeuedElement_;
    Common::Semaphore semaphore_;
    mutable std::mutex mutex_;
};

template <typename T>
ConcreteQueue<T>::ConcreteQueue() :
    queue_(),
    dequeuedElement_(nullptr),
    semaphore_(0),
    mutex_()
{
}

template <typename T>
ConcreteQueue<T>::~ConcreteQueue()
{
}

template <typename T>
void ConcreteQueue<T>::enqueue(const T& element)
{
    {
        std::lock_guard<std::mutex> lock{mutex_};
        queue_.push_back(element);
    }
    semaphore_.notify();
}

template <typename T>
bool ConcreteQueue<T>::isEmpty() const
{
    std::lock_guard<std::mutex> lock{mutex_};
    if (dequeuedElement_)
    {
        // If the single element is already dequeued,
        // the queue is effectively empty
        return queue_.size() < 2;
    }
    return queue_.empty();
}

template <typename T>
const T& ConcreteQueue<T>::dequeue()
{
    if (dequeuedElement_)
    {
        // Release previously dequeued element
        std::lock_guard<std::mutex> lock{mutex_};
        queue_.pop_front();
        dequeuedElement_ = nullptr;
    }

    while (queue_.empty())
    {
        semaphore_.wait();
    }

    std::lock_guard<std::mutex> lock{mutex_};
    dequeuedElement_ = &queue_.front();
    return *dequeuedElement_;
}

} // Data
