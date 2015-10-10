#ifndef DATA_CONFIGURATION_H_
#define DATA_CONFIGURATION_H_

#include "ConfigurationIf.hpp"
#include <string>
#include <map>

namespace Data {

/** Configuration implementation using a std::map to store the items. */
class Configuration final : public ConfigurationIf
{
public:
    Configuration();
    virtual ~Configuration();

    Configuration(const Configuration& rhs);
    Configuration& operator=(const Configuration& rhs);

    virtual bool load(const std::string& key, SerializableIf& item) const override;
    virtual bool save(const std::string& key, const SerializableIf& item) override;
    virtual bool hasItem(const std::string& key) const override;
    virtual void removeItem(const std::string& key) override;
    virtual void clearItems() override;
    
private:
    std::map<std::string, std::string> items_;
};

} // Data

#endif
