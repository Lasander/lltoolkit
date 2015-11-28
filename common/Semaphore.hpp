#ifndef COMMON_SEMAPHORE_HPP_
#define COMMON_SEMAPHORE_HPP_

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace Common {

/** Semaphore, an atomic counter. */
class Semaphore
{
public:
    /** Initialize with @p count resources */
    explicit Semaphore(size_t count = 0);

    /** Prevent copy, assignment and move */
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore& operator=(Semaphore&&) = delete;

    /** Notify @p count resources */
    void notify(size_t count = 1);

    /** Block to wait for @p count resources */
    void wait(size_t count = 1);

    /** Block to wait for a @p duration for @p count resources */
    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& duration, size_t count = 1);

    /** Block to wait until a @p timePoint for @p count resources */
    template<class Clock, class Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& timePoint, size_t count = 1);

    /** @return current resources count */
    size_t getCount() const;

private:
    size_t count_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};

inline Semaphore::Semaphore(size_t count)
    : count_{count}
{}

inline void Semaphore::notify(size_t count)
{
    {
        std::lock_guard<std::mutex> lock{mutex_};
        count_ += count;
    }
    condition_.notify_one();
}

inline void Semaphore::wait(size_t count)
{
    std::unique_lock<std::mutex> lock{mutex_};
    condition_.wait(lock, [&]{ return count_ >= count; });
    count_ -= count;
}

template<class Rep, class Period>
bool Semaphore::wait_for(const std::chrono::duration<Rep, Period>& duration, size_t count)
{
    std::unique_lock<std::mutex> lock{mutex_};
    auto finished = condition_.wait_for(lock, duration, [&]{ return count_ >= count; });

    if (finished)
    {
        count_ -= count;
    }

    return finished;
}

template<class Clock, class Duration>
bool Semaphore::wait_until(const std::chrono::time_point<Clock, Duration>& timePoint, size_t count)
{
    std::unique_lock<std::mutex> lock{mutex_};
    auto finished = condition_.wait_until(lock, timePoint, [&]{ return count_ >= count; });

    if (finished)
        count_ -= count;

    return finished;
}

inline size_t Semaphore::getCount() const
{
    std::lock_guard<std::mutex> lock{mutex_};
    return count_;
}

} // Common

#endif
