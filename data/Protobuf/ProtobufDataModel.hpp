#pragma once

#include "../SerializableDataModel.hpp"
#include "ProtobufSerializer.hpp"

namespace Data {
namespace Protobuf {

/**
 * SerializableDataModelIf implementation that uses a SerializableDataModel<DataType, Less>
 * and a ProtobufSerializer<DataType>.
 */
template <typename DataType, typename Less = std::less<DataType>>
class ProtobufDataModel : public SerializableDataModelIf<DataType>
{
public:
    ProtobufDataModel();
    virtual ~ProtobufDataModel();

    /**
     * @defgroup SerializableDataModelIf implementation
     * @{
     */
    virtual void set(const DataType& data) override;
    virtual const DataType& get() const override;
    virtual Publisher<DataType>& publisher() override;

    virtual bool serialize(std::ostream& output) const override;
    virtual bool deserialize(std::istream& input) override;
    virtual void deserializationComplete() override;
    /**@}*/

private:
    ProtobufDataModel(const ProtobufDataModel&) = delete;
    ProtobufDataModel& operator=(const ProtobufDataModel&) = delete;

    Protobuf::ProtobufSerializer<DataType> serializer_;
    SerializableDataModel<DataType, Less> serializableModel_;
};

template <typename DataType, typename Less>
ProtobufDataModel<DataType, Less>::ProtobufDataModel() :
    serializableModel_(serializer_)
{
}

template <typename DataType, typename Less>
ProtobufDataModel<DataType, Less>::~ProtobufDataModel()
{
}

template <typename DataType, typename Less>
void ProtobufDataModel<DataType, Less>::set(const DataType& data)
{
    serializableModel_.set(data);
}
template <typename DataType, typename Less>
const DataType& ProtobufDataModel<DataType, Less>::get() const
{
    return serializableModel_.get();
}
template <typename DataType, typename Less>
Publisher<DataType>& ProtobufDataModel<DataType, Less>::publisher()
{
    return serializableModel_.publisher();
}

template <typename DataType, typename Less>
bool ProtobufDataModel<DataType, Less>::serialize(std::ostream& output) const
{
    return serializableModel_.serialize(output);
}
template <typename DataType, typename Less>
bool ProtobufDataModel<DataType, Less>::deserialize(std::istream& input)
{
    return serializableModel_.deserialize(input);
}
template <typename DataType, typename Less>
void ProtobufDataModel<DataType, Less>::deserializationComplete()
{
    return serializableModel_.deserializationComplete();
}

} /* namespace Protobuf */
} /* namespace Data */
