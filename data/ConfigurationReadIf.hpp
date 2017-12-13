#pragma once

#include <iosfwd>

namespace Data {

// Forward declarations
class SerializableIf;

/** Configuration provides a storage to save items */
class ConfigurationReadIf
{
public:
    /**
     Load previously saved item identified by @p key to the given @p item.
     @return True if item was successfully loaded
     */
    virtual bool load(const std::string& key, SerializableIf& item) const = 0;

    /** @return true if this configuration contains item with @p key */
    virtual bool hasItem(const std::string& key) const = 0;

    virtual ~ConfigurationReadIf() {}
};

} // Data
