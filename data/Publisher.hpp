#pragma once

#include <map>
#include <functional>
#include <iostream>

namespace Data {

/**
 * Publisher of given data change notifications to a set of subscribers.
 *
 * Subscribers are identified with an object reference and must
 * provide a function:
 * - with empty signature,
 * - taking in the data as const reference, or
 * - taking in the data by value (copy)
 *
 * @tparam DataType The data type
 */
template <typename DataType>
class Publisher
{
public:
    Publisher();
    ~Publisher();

    /**
     * Subscribe to receive change notifications to a member function. If function takes
     * a parameter the data is passed in.
     *
     * @tparam SubscriberType Type of the subscriber object
     * @tparam InterfaceType Interface type where the callback member function can be found.
     * 		   Needs to be either Subscriber or its base class.
     * @tparam ReturnType Callback member function return type.
     *
     * Note: all template parameters are deduced from the argument types.
     *
     * @param object Object to call
     * @param callback Member function to call
     */
    template <typename SubscriberType, typename InterfaceType, typename ReturnType>
    bool subscribe(SubscriberType& object, ReturnType (InterfaceType::*callback)(const DataType&));
    template <typename SubscriberType, typename InterfaceType, typename ReturnType>
    bool subscribe(SubscriberType& object, ReturnType (InterfaceType::*callback)(DataType));
    template <typename SubscriberType, typename InterfaceType, typename ReturnType>
    bool subscribe(SubscriberType& object, ReturnType (InterfaceType::*callback)());

    /**
     * Subscribe to receive change notifications to a free function. If function takes
     * a parameter the data is passed in.
     *
     * @tparam SubscriberType Type of the subscriber object
     * @tparam ReturnType Callback function return type.
     *
     * @param object Object used as identifier of the subscription
     * @param callback Function to call
     */
    template <typename SubscriberType, typename ReturnType>
    bool subscribe(SubscriberType& object, ReturnType (*callback)(const DataType&));
    template <typename SubscriberType, typename ReturnType>
    bool subscribe(SubscriberType& object, ReturnType (*callback)(DataType));
    template <typename SubscriberType, typename ReturnType>
    bool subscribe(SubscriberType& object, ReturnType (*callback)());

    /**
     * Unsubscribe from change notifications.
     */
    template <typename SubscriberType>
    bool unsubscribe(SubscriberType& object);

    /**
     * Notify subscribers of data change. Should be called by the data owner
     * when the data changes.
     *
     * @param data Updated data
     */
    void notifySubscribers(const DataType& data);

private:
    /** Prevent copy, move and assignment */
    Publisher(const Publisher&);
    Publisher(Publisher&&);
    Publisher& operator=(const Publisher&);

    using NotificationFunction = const std::function<void(const DataType&)>;
    using SubscriberMap = std::map<void*, NotificationFunction>;
    SubscriberMap subscribers_;

    /**
     * Add new subscriber.
     *
     * @param object Identifier for the subscriber
     * @param notificationFunction Function used to notify
     * @return True if the subscription was successful, false in case of duplicate subscriber
     */
    bool addSubscriber(void* object, const NotificationFunction& notificationFunction);

    /**
     * Remove existing subscriber.
     *
     * @param object Identifier for the subscriber
     * @return True if the unsubscription was successful, false if the subscriber did not exist
     */
    bool removeSubscriber(void* object);
};

template <typename DataType>
template <typename SubscriberType, typename InterfaceType, typename ReturnType>
bool Publisher<DataType>::subscribe(SubscriberType& object, ReturnType (InterfaceType::*callback)(const DataType&))
{
    const std::function<void(const DataType&)> f = [&object, callback](const DataType& data) { (object.*callback)(data); };
    return addSubscriber(&object, f);
}

template <typename DataType>
template <typename SubscriberType, typename InterfaceType, typename ReturnType>
bool Publisher<DataType>::subscribe(SubscriberType& object, ReturnType (InterfaceType::*callback)(DataType))
{
    std::function<void(const DataType&)> f = [&object, callback](const DataType& data) { (object.*callback)(data); };
    return addSubscriber(&object, f);
}

template <typename DataType>
template <typename SubscriberType, typename InterfaceType, typename ReturnType>
bool Publisher<DataType>::subscribe(SubscriberType& object, ReturnType (InterfaceType::*callback)())
{
    std::function<void(const DataType&)> f = [&object, callback](const DataType& data) { (object.*callback)(); };
    return addSubscriber(&object, f);
}

template <typename DataType>
template <typename SubscriberType, typename ReturnType>
bool Publisher<DataType>::subscribe(SubscriberType& object, ReturnType (*callback)(const DataType&))
{
    std::function<void(const DataType&)> f = [callback](const DataType& data) { callback(data); };
    return addSubscriber(&object, f);
}

template <typename DataType>
template <typename SubscriberType, typename ReturnType>
bool Publisher<DataType>::subscribe(SubscriberType& object, ReturnType (*callback)(DataType))
{
    std::function<void(const DataType&)> f = [callback](const DataType& data) { callback(data); };
    return addSubscriber(&object, f);
}

template <typename DataType>
template <typename SubscriberType, typename ReturnType>
bool Publisher<DataType>::subscribe(SubscriberType& object, ReturnType (*callback)())
{
    std::function<void(const DataType&)> f = [callback](const DataType& data) { callback(); };
    return addSubscriber(&object, f);
}

template <typename DataType>
template <typename Object>
bool Publisher<DataType>::unsubscribe(Object& object)
{
    return removeSubscriber(&object);
}

template <typename DataType>
Publisher<DataType>::Publisher()
{
}

template <typename DataType>
Publisher<DataType>::~Publisher()
{
}

template <typename DataType>
void Publisher<DataType>::notifySubscribers(const DataType& data)
{
    for (auto& sub : subscribers_)
        sub.second(data);
}

template <typename DataType>
bool Publisher<DataType>::addSubscriber(void* object, const NotificationFunction& notifyFunction)
{
    if (!subscribers_.emplace(object, notifyFunction).second)
    {
        std::cerr << "Failed to add subscriber: duplicate" << std::endl;
        return false;
    }
    return true;
}

template <typename DataType>
bool Publisher<DataType>::removeSubscriber(void* object)
{
    if (subscribers_.erase(object) < 1)
    {
    	std::cerr << "Failed to remove subscriber: non-existent" << std::endl;
        return false;
    }
    return true;
}

} // Data
