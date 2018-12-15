#pragma once

#include "ConfigurationIf.hpp"
#include <map>
#include <string>

namespace Data {

/** Configuration implementation using a std::map to store the items. */
class Configuration final : public ConfigurationIf
{
public:
    Configuration();
    ~Configuration() override;

    Configuration(const Configuration& rhs);
    Configuration& operator=(const Configuration& rhs);

    bool load(const std::string& key, SerializableIf& item) const override;
    bool save(const std::string& key, const SerializableIf& item) override;
    bool hasItem(const std::string& key) const override;
    void removeItem(const std::string& key) override;
    void clearItems() override;

private:
    std::map<std::string, std::string> items_;
};

} // namespace Data
