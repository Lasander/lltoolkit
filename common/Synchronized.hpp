#pragma once

#include <mutex>

namespace Common {

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
 */
template <typename Data, typename Lock = std::mutex>
class Synchronized : private Data
{
public:
    /** Constructor template to add a lock parameter to the Data construction */
    template <typename ...Args>
    Synchronized(Lock& lock, Args... argsP);

    /** Arrow operator to conveniently perform single call transactions */
    auto operator->();

    /** Operator to conveniently perform single call transactions */
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

    Lock& lock_;
};


template <typename Data, typename Lock>
template <typename ...Args>
Synchronized<Data, Lock>::Synchronized(Lock& lock, Args... argsP)
  : Data(argsP...),
    lock_(lock)
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
    obj_->lock_.lock();
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
        obj_->lock_.unlock();
    }
}

template <typename Data, typename Lock>
Data* Synchronized<Data, Lock>::Transaction::operator->()
{
    return obj_;
}

} // Common
