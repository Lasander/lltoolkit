#pragma once

#include <mutex>

namespace Common {

/** Synchronized data lock policies. */
///@{

/** Original data class must implement lock() and unlock() itself */
class DataLock
{
};

/** A new lock is created by Synchronized class */
template <typename Lock>
class InternalLock
{
public:
    InternalLock() : lock_() {}
    void lock() { lock_.lock(); }
    void unlock() { lock_.unlock(); }

private:
    Lock lock_;
};

/** An external lock is provided to Synchronized class */
template <typename Lock>
class ExternalLock
{
public:
    ExternalLock(Lock& lock) : lock_(lock) {}
    void lock() { lock_.lock(); }
    void unlock() { lock_.unlock(); }

private:
    Lock& lock_;
};

///@}

/**
 * A generic synchronization wrapper
 *
 * Controls the access to the data, by allowing it only when a lock is held.
 * A transaction object is provided to make multiple calls while holding.
 *
 * Example:
 *
 *   // Before
 *   MyData d;
 *   d.doThis();
 *   d.doThat();
 *
 *   // After
 *   Synchronized<MyData> d;
 *   d->doThis(); // Single locked call
 *
 *   {
 *       auto t = d.makeTransaction();
 *       // Holding the lock until t goes out of scope
 *       t->doThis();
 *       t->doThat();
 *   }
 *
 * @tparam Data Wrapped data type
 * @tparam Lock Lock policy
 */
template <typename Data, typename Lock = InternalLock<std::mutex>>
class Synchronized : private Data, private Lock
{
public:
    /** Constructor template to add a lock parameter to the Data construction */
    template <typename ...Args>
    Synchronized(Lock& lock, Args... args);

    /** Constructor template to construction data and use internal lock */
    template <typename ...Args>
    Synchronized(Args... args);

    /** Arrow operator to conveniently perform single call transactions */
    auto operator->();

    /** @return Transaction object to perform a series of calls under a single lock/unlock */
    auto makeTransaction();

private:
    /** Transaction to perform multiple operations as an atomic transaction */
    class Transaction
    {
    public:
        // Transaction can be moved to allow return by value @see Synchronized::makeTransaction
        Transaction(Transaction&& other);

        ~Transaction();

        /** Get access to the data and its original operations */
        Data* operator->();

    private:
        friend class Synchronized;

        // Can be constructed only by class Synchronized
        Transaction(Synchronized& obj);

        // No copying
        Transaction(const Transaction&) = delete;
        Transaction& operator=(const Transaction&) = delete;

        Synchronized* obj_;
    };
};


template <typename Data, typename Lock>
template <typename ...Args>
Synchronized<Data, Lock>::Synchronized(Lock& lock, Args... args)
  : Data(args...),
    Lock(lock)
{
}

template <typename Data, typename Lock>
template <typename ...Args>
Synchronized<Data, Lock>::Synchronized(Args... args)
  : Data(args...),
    Lock()
{
}

template <typename Data, typename Lock>
auto Synchronized<Data, Lock>::operator->()
{
    return makeTransaction();
}

template <typename Data, typename Lock>
auto Synchronized<Data, Lock>::makeTransaction()
{
    return Transaction(*this);
}

template <typename Data, typename Lock>
Synchronized<Data, Lock>::Transaction::Transaction(Synchronized& obj)
  : obj_(&obj)
{
    obj_->lock();
}

template <typename Data, typename Lock>
Synchronized<Data, Lock>::Transaction::Transaction(Transaction&& other)
  : obj_(other.obj_)
{
    other.obj_ = nullptr;
}

template <typename Data, typename Lock>
Synchronized<Data, Lock>::Transaction::~Transaction()
{
    if (obj_)
    {
        obj_->unlock();
    }
}

template <typename Data, typename Lock>
Data* Synchronized<Data, Lock>::Transaction::operator->()
{
    return obj_;
}

} // Common
