#ifndef DATA_SERIALIZABLEDATAMODELIF_HPP_
#define DATA_SERIALIZABLEDATAMODELIF_HPP_

#include "SerializableIf.hpp"
#include "DataModelIf.hpp"

namespace Data {

/** Interface for serializable data models */
template <typename DataType>
class SerializableDataModelIf :
    public SerializableIf,
    public DataModelIf<DataType>
{
public:
    virtual ~SerializableDataModelIf() {}
};

} // Data

#endif
