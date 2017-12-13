#pragma once

#include "DataReadIf.hpp"
#include "DataWriteIf.hpp"

namespace Data {

/** Interface to read and write data of given DataType */
template <typename DataType>
class DataIf : public DataReadIf<DataType>, public DataWriteIf<DataType>
{
public:
    virtual ~DataIf() {}

protected:
    DataIf() {}
};

} // namespace Data
