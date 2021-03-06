#pragma once

#include "ConfigurationReadIf.hpp"
#include "ConfigurationWriteIf.hpp"

namespace Data {

/** Configuration provides a storage to save items */
class ConfigurationIf : public ConfigurationReadIf, public ConfigurationWriteIf
{
public:
    ~ConfigurationIf() override {}
};

} // namespace Data
