#pragma once

#include <iosfwd>

namespace Data {

// Forward declarations
class SerializableIf;

/** Configuration provides a storage to save items */
class ConfigurationWriteIf
{
public:
    /**
     Save (store) @p item with @p key.
     Note that any previously saved item with the same key is overwritten.
     @return true if item was successfully saved.
     */
    virtual bool save(const std::string& key, const SerializableIf& item) = 0;

    virtual void removeItem(const std::string& key) = 0;
    virtual void clearItems() = 0;

    virtual ~ConfigurationWriteIf() {}
};

} // Data
