#pragma once

#include <mutex>

namespace Common {

/** Synchronized data lock policies. */
///@{

/** Original data class must implement lock() and unlock() itself */
template <typename Data>
class DataLock
{
public:
    void lock(Data* data) { data->lock(); }
    void unlock(Data* data) { data->unlock(); }
};

/** A new lock is created by Synchronized class */
template <typename Lock, typename Data = void>
class InternalLock
{
public:
    InternalLock() : lock_() {}
    void lock(Data*) { lock_.lock(); }
    void unlock(Data*) { lock_.unlock(); }

private:
    Lock lock_;
};

/** An external lock is provided to Synchronized class */
template <typename Lock, typename Data = void>
class ExternalLock
{
public:
    ExternalLock(Lock& lock) : lock_(lock) {}
    ExternalLock(ExternalLock&& other) = default;

    void lock(Data*) { lock_.lock(); }
    void unlock(Data*) { lock_.unlock(); }

private:
    ExternalLock(const ExternalLock& other) = delete;
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
    /** Constructor template to copy a lock in addition to data construction */
    template <typename ...Args>
    Synchronized(const Lock& lock, Args... args);

    /** Constructor template to move a lock in addition to data construction */
    template <typename ...Args>
    Synchronized(Lock&& lock, Args... args);

    /** Constructor template to default construct a lock in addition to data construction */
    template <typename ...Args>
    Synchronized(Args... args);

    /** Arrow operator to conveniently perform single call transactions */
    auto operator->();
    auto operator->() const;

    /** @return Transaction object to perform a series of calls under a single lock/unlock */
    auto makeTransaction();
    auto makeTransaction() const;

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

    /** Transaction to perform multiple operations as an atomic transaction to a const data */
    class ConstTransaction
    {
    public:
        // Transaction can be moved to allow return by value @see Synchronized::makeTransaction
        ConstTransaction(ConstTransaction&& other);

        ~ConstTransaction();

        /** Get access to the data and its original operations */
        const Data* operator->();

    private:
        friend class Synchronized;

        // Can be constructed only by class Synchronized
        ConstTransaction(const Synchronized& obj);

        // No copying
        ConstTransaction(const ConstTransaction&) = delete;
        ConstTransaction& operator=(const ConstTransaction&) = delete;

        const Synchronized* obj_;
    };
};

// Synchronized implementation
template <typename Data, typename Lock>
template <typename ...Args>
Synchronized<Data, Lock>::Synchronized(const Lock& lock, Args... args)
  : Data(args...),
    Lock(lock)
{
}

template <typename Data, typename Lock>
template <typename ...Args>
Synchronized<Data, Lock>::Synchronized(Lock&& lock, Args... args)
  : Data(args...),
    Lock(std::forward<Lock>(lock))
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
auto Synchronized<Data, Lock>::operator->() const
{
    return makeTransaction();
}

template <typename Data, typename Lock>
auto Synchronized<Data, Lock>::makeTransaction()
{
    return Transaction(*this);
}

template <typename Data, typename Lock>
auto Synchronized<Data, Lock>::makeTransaction() const
{
    return ConstTransaction(*this);
}

// Synchronized::Transaction implementation
template <typename Data, typename Lock>
Synchronized<Data, Lock>::Transaction::Transaction(Synchronized& obj)
  : obj_(&obj)
{
    obj_->Lock::lock(obj_);
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
        obj_->Lock::unlock(obj_);
    }
}

template <typename Data, typename Lock>
Data* Synchronized<Data, Lock>::Transaction::operator->()
{
    return obj_;
}

// Synchronized::ConstTransaction implementation
template <typename Data, typename Lock>
Synchronized<Data, Lock>::ConstTransaction::ConstTransaction(const Synchronized& obj)
  : obj_(&obj)
{
    Synchronized* nonConstData = const_cast<Synchronized*>(obj_);
    nonConstData->Lock::lock(nonConstData);
}

template <typename Data, typename Lock>
Synchronized<Data, Lock>::ConstTransaction::ConstTransaction(ConstTransaction&& other)
  : obj_(other.obj_)
{
    other.obj_ = nullptr;
}

template <typename Data, typename Lock>
Synchronized<Data, Lock>::ConstTransaction::~ConstTransaction()
{
    if (obj_)
    {
        Synchronized* nonConstData = const_cast<Synchronized*>(obj_);
        nonConstData->Lock::unlock(nonConstData);
    }
}

template <typename Data, typename Lock>
const Data* Synchronized<Data, Lock>::ConstTransaction::operator->()
{
    return obj_;
}

} // Common
