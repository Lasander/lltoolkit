#pragma once

#include "DataModelIf.hpp"
#include "SerializableIf.hpp"

namespace Data {

/** Interface for serializable data models */
template <typename DataType>
class SerializableDataModelIf : public SerializableIf, public DataModelIf<DataType>
{
public:
    virtual ~SerializableDataModelIf() {}
};

} // namespace Data
