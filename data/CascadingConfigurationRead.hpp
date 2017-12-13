#pragma once

#include "ConfigurationIf.hpp"

namespace Data {

/**
 * Configuration item creating a hierarchy.
 *
 * Links a configuration and its parent. Items are first searched from the configuration
 * and if not found, the search is redirected to the parent.
 */
class CascadingConfigurationRead : public ConfigurationReadIf
{
public:
    /**
     * Construct CascadingConfigurationRead
     */
    CascadingConfigurationRead(
        const ConfigurationReadIf& configuration,
        const ConfigurationReadIf& parentConfiguration);

    virtual ~CascadingConfigurationRead();

    /** @defgroup ConfigurationReadIf implementation */
    ///@{
    virtual bool load(const std::string& key, SerializableIf& item) const override;
    virtual bool hasItem(const std::string& key) const override;
    ///@}

private:
	CascadingConfigurationRead(const CascadingConfigurationRead&);
	CascadingConfigurationRead& operator=(const CascadingConfigurationRead&);

    const ConfigurationReadIf& configuration_;
    const ConfigurationReadIf& parentConfiguration_;
};

} // namespace Data
