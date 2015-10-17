#ifndef DATA_CONFIGURATIONIF_H_
#define DATA_CONFIGURATIONIF_H_

#include "ConfigurationReadIf.hpp"
#include "ConfigurationWriteIf.hpp"

namespace Data {

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
