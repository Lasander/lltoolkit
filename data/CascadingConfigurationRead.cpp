#include "CascadingConfigurationRead.hpp"

namespace Data {

CascadingConfigurationRead::CascadingConfigurationRead(
    const ConfigurationReadIf& configuration,
    const ConfigurationReadIf& parentConfiguration) :
    configuration_(configuration),
    parentConfiguration_(parentConfiguration)
{
}

CascadingConfigurationRead::~CascadingConfigurationRead()
{
}

bool CascadingConfigurationRead::load(const std::string& key, SerializableIf& item) const
{
    if (configuration_.hasItem(key))
    {
        return configuration_.load(key, item);
    }

    return parentConfiguration_.load(key, item);
}

bool CascadingConfigurationRead::hasItem(const std::string& key) const
{
    return configuration_.hasItem(key) || parentConfiguration_.hasItem(key);
}

} // Data
