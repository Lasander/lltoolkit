#ifndef DATA_CONFIGURATIONIF_H_
#define DATA_CONFIGURATIONIF_H_

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
	virtual bool hasItem(const std::string& key) const = 0;
    
    virtual ~ConfigurationReadIf() {}
};

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


/** Configuration provides a storage to save items */
class ConfigurationIf :
	public ConfigurationReadIf,
    public ConfigurationWriteIf
{
public:
    virtual ~ConfigurationIf() {}
};

} // Data

#endif
