#ifndef DATA_PROTOBUF_PROTOBUFSERIALIZER_HPP_
#define DATA_PROTOBUF_PROTOBUFSERIALIZER_HPP_

#include "../SerializerIf.hpp"
#include <iosfwd>

namespace Data {
namespace Protobuf {

/** Serializer for protocol buffers */
template <typename DataType>
class ProtobufSerializer : public SerializerIf<DataType>
{
public:
    /**
     * @defgroup SerializerIf implementation
     * @{
     */
    virtual bool serialize(const DataType& data, std::ostream& output) const override;
    virtual bool deserialize(DataType& data, std::istream& input) const override;
    /**@}*/

    virtual ~ProtobufSerializer() {}
};

template <typename DataType>
bool ProtobufSerializer<DataType>::serialize(const DataType& data, std::ostream& output) const
{
    return data.SerializeToOstream(&output);
}

template <typename DataType>
bool ProtobufSerializer<DataType>::deserialize(DataType& data, std::istream& input) const
{
    return data.ParseFromIstream(&input);
}

} /* namespace Protobuf */
} /* namespace Data */

#endif
