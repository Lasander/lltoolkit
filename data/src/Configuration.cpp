#include "data/Configuration.hpp"
#include "data/SerializableIf.hpp"
#include <iostream>
#include <sstream>

namespace Data {

Configuration::Configuration() {}

Configuration::~Configuration() {}

Configuration::Configuration(const Configuration& rhs) :
    items_(rhs.items_)
{
}

Configuration& Configuration::operator=(const Configuration& rhs)
{
    if (this != &rhs)
    {
        items_ = rhs.items_;
    }

    return *this;
}

bool Configuration::load(const std::string& key, SerializableIf& item) const
{
    auto iter = items_.find(key);
    if (iter != items_.cend())
    {
        std::istringstream stream(iter->second);
        return item.deserialize(stream);
    }
    return false;
}

bool Configuration::save(const std::string& key, const SerializableIf& item)
{
    std::ostringstream stream;
    if (item.serialize(stream))
    {
        items_[key] = stream.str();
        return true;
    }

    return false;
}

bool Configuration::hasItem(const std::string& key) const
{
    return items_.find(key) != items_.cend();
}

void Configuration::removeItem(const std::string& key)
{
    items_.erase(key);
}

void Configuration::clearItems()
{
    items_.clear();
}

} // Data
