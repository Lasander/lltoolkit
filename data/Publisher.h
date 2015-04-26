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
public:
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (Object::*callback)(const DataType&));
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (Object::*callback)(DataType));
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (Object::*callback)());

    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (*callback)(const DataType&));
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (*callback)(DataType));
    template <typename Object, typename ReturnType>
    bool subscribe(Object& object, ReturnType (*callback)());

    template <typename Object>
    bool unsubscribe(Object& object);

protected:
    void notifySubscribers(const DataType& data);

private:
    class Callback
    {
        std::function<void(const DataType&)> function_;
    public:
        Callback(std::function<void(const DataType&)> function);
        template <typename R> Callback(std::function<R(const DataType&)> function);
        ~Callback();

        void operator()(const DataType& data);
    };

    using SubscriberMap = std::map<void*, std::shared_ptr<Callback>>;
    SubscriberMap subscribers_;

    bool addSubscriber(void* object, std::shared_ptr<Callback> callable);
    bool removeSubscriber(void* object);

private:

};

template <typename DataType>
template <typename Object, typename ReturnType>
bool Publisher<DataType>::subscribe(Object& object, ReturnType (Object::*callback)(const DataType&))
{
    std::function<ReturnType(const DataType&)> f = std::bind(callback, &object, std::placeholders::_1);
    return addSubscriber(&object, std::make_shared<Callback>(f));
}

template <typename DataType>
template <typename Object, typename ReturnType>
bool Publisher<DataType>::subscribe(Object& object, ReturnType (Object::*callback)(DataType))
{
    std::function<ReturnType(const DataType&)> f = std::bind(callback, &object, std::placeholders::_1);
    return addSubscriber(&object, std::make_shared<Callback>(f));
}

template <typename DataType>
template <typename Object, typename ReturnType>
bool Publisher<DataType>::subscribe(Object& object, ReturnType (Object::*callback)())
{
    std::function<ReturnType(const DataType&)> f = std::bind(callback, &object);
    return addSubscriber(&object, std::make_shared<Callback>(f));
}

template <typename DataType>
template <typename Object, typename ReturnType>
bool Publisher<DataType>::subscribe(Object& object, ReturnType (*callback)(const DataType&))
{
    return addSubscriber(&object, std::make_shared<Callback>(callback));
}

template <typename DataType>
template <typename Object, typename ReturnType>
bool Publisher<DataType>::subscribe(Object& object, ReturnType (*callback)(DataType))
{
    std::function<ReturnType(const DataType&)> f = std::bind(callback, std::placeholders::_1);
    return addSubscriber(&object, std::make_shared<Callback>(f));
}

template <typename DataType>
template <typename Object, typename ReturnType>
bool Publisher<DataType>::subscribe(Object& object, ReturnType (*callback)())
{
    std::function<ReturnType(const DataType&)> f = std::bind(callback);
    return addSubscriber(&object, std::make_shared<Callback>(f));
}

template <typename DataType>
template <typename Object>
bool Publisher<DataType>::unsubscribe(Object& object)
{
    return removeSubscriber(&object);
}

template <typename DataType>
void Publisher<DataType>::notifySubscribers(const DataType& data)
{
    for (auto& sub : subscribers_)
        sub.second->operator()(data);
}

template <typename DataType>
bool Publisher<DataType>::addSubscriber(void* object, std::shared_ptr<Callback> callable)
{
    const typename SubscriberMap::value_type entry{object, callable};
    if (!subscribers_.insert(entry).second)
    {
        std::fprintf(stderr, "Failed to add subscriber: duplicate\n");
        return false;
    }
    return true;
}

template <typename DataType>
bool Publisher<DataType>::removeSubscriber(void* object)
{
    if (subscribers_.erase(object) < 1)
    {
        std::fprintf(stderr, "Failed to remove subscriber: non-existent\n");
        return false;
    }
    return true;
}

template <typename DataType>
Publisher<DataType>::Callback::Callback(std::function<void(const DataType&)> function)
  : function_(function)
{
}

template <typename DataType>
template <typename R>
Publisher<DataType>::Callback::Callback(std::function<R(const DataType&)> function)
  : Callback([=](const DataType& data) -> void { (void)function(data); })
{
}

template <typename DataType>
Publisher<DataType>::Callback::~Callback() {}

template <typename DataType>
void Publisher<DataType>::Callback::operator()(const DataType& data)
{
    function_(data);
}

} // Data

#endif /* DATA_PUBLISHER_H_ */
