#ifndef DATA_CASCADINGCONFIGURATIONREAD_HPP_
#define DATA_CASCADINGCONFIGURATIONREAD_HPP_

#include "ConfigurationIf.hpp"

namespace Data {

/**  */
class CascadingConfigurationRead : public ConfigurationReadIf
{
public:
    CascadingConfigurationRead(
        const ConfigurationReadIf& wrappedConfiguration,
        const ConfigurationReadIf& parentConfiguration);

    virtual ~CascadingConfigurationRead();

    virtual bool load(const std::string& key, SerializableIf& item) const override;
    virtual bool hasItem(const std::string& key) const override;
private:
	CascadingConfigurationRead(const CascadingConfigurationRead&);
	CascadingConfigurationRead& operator=(const CascadingConfigurationRead&);

    const ConfigurationReadIf& wrappedConfiguration_;
    const ConfigurationReadIf& parentConfiguration_;
};

}

#endif
