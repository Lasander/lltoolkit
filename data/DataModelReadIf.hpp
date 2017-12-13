#pragma once

#include "DataReadIf.hpp"

namespace Data {

// Publisher type
template <typename> class Publisher;

/** Interface to read and subscribe to data of given DataType */
template <typename DataType>
class DataModelReadIf : public DataReadIf<DataType>
{
public:
    /** @return publisher */
	virtual Publisher<DataType>& publisher() = 0;

    virtual ~DataModelReadIf() {}

protected:
    DataModelReadIf() {}
};

}
