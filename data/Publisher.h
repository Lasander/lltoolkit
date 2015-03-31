/**
 * Publisher.h
 *
 *  Created on: Mar 31, 2015
 *      Author: lasse
 */

#ifndef DATA_PUBLISHER_H_
#define DATA_PUBLISHER_H_

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
    class FunctionCallback : public CallbackIf
    {
        std::function<R(const DataType&)> function_;
    public:
        FunctionCallback(std::function<R(const DataType& data)> function)
          : function_(function)
        {
        }
        FunctionCallback(std::function<R(Args...)> function)
          : function_(std::bind(function))
        {
        }
        virtual ~FunctionCallback() {}

        virtual void operator()(const DataType& data)
        {
            function_(data);
        }
    };

    using SubscriberMap = std::map<void*, std::shared_ptr<CallbackIf>>;
    SubscriberMap subscribers_;

public:
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (Object::*callback)(const DataType&))
    {
        std::function<ReturnType(const DataType&)> f = std::bind(callback, &object, std::placeholders::_1);
        return addSubscriber(&object, std::make_shared<FunctionCallback<ReturnType>>(f));
    }
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (Object::*callback)(DataType))
    {
        std::function<ReturnType(const DataType&)> f = std::bind(callback, &object, std::placeholders::_1);
        return addSubscriber(&object, std::make_shared<FunctionCallback<ReturnType>>(f));
    }
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (Object::*callback)())
    {
        std::function<ReturnType(const DataType&)> f = std::bind(callback, &object);
        return addSubscriber(&object, std::make_shared<FunctionCallback<ReturnType>>(f));
    }

    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (*callback)(const DataType&))
    {
        return addSubscriber(&object, std::make_shared<FunctionCallback<ReturnType>>(callback));
    }
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (*callback)(DataType))
    {
        std::function<ReturnType(const DataType&)> f = std::bind(callback, std::placeholders::_1);
        return addSubscriber(&object, std::make_shared<FunctionCallback<ReturnType>>(f));
    }
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (*callback)())
    {
        std::function<ReturnType(const DataType&)> f = std::bind(callback);
        return addSubscriber(&object, std::make_shared<FunctionCallback<ReturnType>>(f));
    }

    template <typename Object>
    bool unsubscribe(Object& object)
    {
        return removeSubscriber(&object);
    }

protected:
    void notifySubscribers(const DataType& data)
    {
        for (auto& sub : subscribers_)
            sub.second->operator()(data);
    }

private:
    bool addSubscriber(void* object, std::shared_ptr<CallbackIf> callable)
    {
        const typename SubscriberMap::value_type entry{object, callable};
        if (!subscribers_.insert(entry).second)
        {
            std::fprintf(stderr, "Failed to add subscriber: duplicate\n");
            return false;
        }
        return true;
    }

    bool removeSubscriber(void* object)
    {
        if (subscribers_.erase(object) < 1)
        {
            std::fprintf(stderr, "Failed to remove subscriber: non-existent\n");
            return false;
        }
        return true;
    }
};

} // Data

#endif /* DATA_PUBLISHER_H_ */
