/**
 * DataModel.hpp
 *
 *  Created on: Mar 29, 2015
 *      Author: lasse
 */

#ifndef DATAMODEL_H_
#define DATAMODEL_H_

#include <map>
#include <functional>
#include <cstdio>

namespace Data {

template <typename DataType>
class Publisher
{
    class CallbackIf
    {
    public:
        virtual void operator()(const DataType& data) = 0;
        virtual ~CallbackIf() {}
    };

    template <typename R, typename... Args>
    class CallableFunction : public CallbackIf
    {
        std::function<R(Args...)> function_;
    public:
        CallableFunction(std::function<R(Args...)> function)
         : function_(function)
        {
        }
        virtual ~CallableFunction() {}

        virtual void operator()(const DataType& /*data*/)
        {
            function_();
        }
    };

    template <typename R, typename... Args>
    class FunctionCallback : public CallbackIf
    {
        std::function<R(const DataType&)> function_;
    public:
        FunctionCallback(std::function<R(Args...)> function)
          : function_(std::bind(function))
        {
            std::fprintf(stderr, "FunctionCallback\n");
        }
        virtual ~FunctionCallback() {}

        virtual void operator()(const DataType& data)
        {
            function_(data);
        }
    };

    using SubscriberMap = std::map<void*, CallbackIf*>;
    SubscriberMap subscribers_;

public:
    template <typename Object, typename ReturnType>
    void subscribe(Object& object, ReturnType (Object::*callback)())
    {
        std::fprintf(stderr, "subscribe\n");
        std::function<ReturnType(void)> f = std::bind(callback, object);
        addSubscriber(&object, new FunctionCallback<ReturnType>(f));
    }

    template <typename Object, typename ReturnType>
    void subscribe(Object& object, std::function<ReturnType()> callback)
    {
        std::fprintf(stderr, "subscribe2\n");
        addSubscriber(&object, new FunctionCallback<ReturnType>(callback));
    }

    template <typename Object>
    void unsubscribe(Object& object)
    {
        removeSubscriber(object);
    }

protected:
    void notifySubscribers(const DataType& data)
    {
        for (auto& sub : subscribers_)
            sub.second->operator()(data);
    }

private:
    void addSubscriber(void* object, CallbackIf* callable)
    {
        const typename SubscriberMap::value_type entry{object, callable};
        if (!subscribers_.insert(entry).second)
        {
            std::fprintf(stderr, "Failed to add subscriber: duplicate\n");
        }
    }

    void removeSubscriber(void* object)
    {
        if (subscribers_.erase(object) < 1)
        {
            std::fprintf(stderr, "Failed to remove subscriber: non-existent\n");
        }
    }
};

template <typename T>
class DataModel : public Publisher<T>
{
    T data_;
    Publisher<T> dataPublisher_;

public:
    DataModel()
     : data_{}
    {
    }

    DataModel(const T& data)
      : data_(data)
    {
    }

    virtual ~DataModel()
    {
    }

    void set(const T& data)
    {
        data_ = data;
        Publisher<T>::notifySubscribers(data_);
    }

    const T& get() const
    {
        return data_;
    }

    T getCopy() const
    {
        return data_;
    }
};

}  // namespace Data

#endif /* DATAMODEL_H_ */
