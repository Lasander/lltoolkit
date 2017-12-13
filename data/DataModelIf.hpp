#pragma once

#include "DataModelReadIf.hpp"
#include "DataWriteIf.hpp"

namespace Data {

/** Interface to read and write data of given DataType */
template <typename DataType>
class DataModelIf : public DataModelReadIf<DataType>, public DataWriteIf<DataType>
{
public:
    virtual ~DataModelIf() {}

protected:
    DataModelIf() {}
};

} // namespace Data
