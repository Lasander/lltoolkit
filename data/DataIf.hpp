#ifndef DATA_DATAIF_HPP_
#define DATA_DATAIF_HPP_

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

}

#endif
