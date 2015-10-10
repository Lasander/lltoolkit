#include "CascadingConfigurationRead.hpp"

namespace Data {

CascadingConfigurationRead::CascadingConfigurationRead(
    const ConfigurationReadIf& wrappedConfiguration,
    const ConfigurationReadIf& parentConfiguration) :
    wrappedConfiguration_(wrappedConfiguration),
    parentConfiguration_(parentConfiguration)
{
}

CascadingConfigurationRead::~CascadingConfigurationRead()
{
}

bool CascadingConfigurationRead::load(const std::string& key, SerializableIf& item) const
{
    if (wrappedConfiguration_.hasItem(key))
    {
        return wrappedConfiguration_.load(key, item);
    }

    return parentConfiguration_.load(key, item);
}

bool CascadingConfigurationRead::hasItem(const std::string& key) const
{
    return wrappedConfiguration_.hasItem(key) || parentConfiguration_.hasItem(key);
}

} // Data
