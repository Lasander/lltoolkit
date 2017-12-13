#pragma once

#include "DataModel.hpp"
#include "SerializableDataModelIf.hpp"
#include "SerializerIf.hpp"

namespace Data {

/** SerializableDataModelIf implementation using a DataModel<DataType, Less> and SerializerIf<DataType> */
template <typename DataType, typename Less = std::less<DataType>>
class SerializableDataModel : public SerializableDataModelIf<DataType>
{
public:
    SerializableDataModel(const SerializerIf<DataType>& serializer);
    virtual ~SerializableDataModel();

    /** @defgroup SerializableDataModelIf implementation */
    ///@{
    virtual void set(const DataType& data) override;
    virtual const DataType& get() const override;
    virtual Publisher<DataType>& publisher() override;

    virtual bool serialize(std::ostream& output) const override;
    virtual bool deserialize(std::istream& input) override;
    virtual void deserializationComplete() override;
    ///@}

private:
    SerializableDataModel(const SerializableDataModel&) = delete;
    SerializableDataModel& operator=(const SerializableDataModel&) = delete;

    const SerializerIf<DataType>& serializer_;
    DataModel<DataType, Less> dataModel_;
};

template <typename DataType, typename Less>
SerializableDataModel<DataType, Less>::SerializableDataModel(const SerializerIf<DataType>& serializer)
  : serializer_(serializer)
{
}

template <typename DataType, typename Less>
SerializableDataModel<DataType, Less>::~SerializableDataModel()
{
}

template <typename DataType, typename Less>
void SerializableDataModel<DataType, Less>::set(const DataType& data)
{
    dataModel_.set(data);
}

template <typename DataType, typename Less>
const DataType& SerializableDataModel<DataType, Less>::get() const
{
    return dataModel_.get();
}

template <typename DataType, typename Less>
Publisher<DataType>& SerializableDataModel<DataType, Less>::publisher()
{
    return dataModel_.publisher();
}

template <typename DataType, typename Less>
bool SerializableDataModel<DataType, Less>::serialize(std::ostream& output) const
{
    return serializer_.serialize(get(), output);
}

template <typename DataType, typename Less>
bool SerializableDataModel<DataType, Less>::deserialize(std::istream& input)
{
    // Deserialize first to a temporary data object and apply to actual data model
    // only if the deserialization was a success
    DataType deserializedData{};
    const bool result = serializer_.deserialize(deserializedData, input);
    if (result)
    {
        // Uses setInternal instead of set to prevent notification, which is sent in deserializationComplete
        dataModel_.setInternal(deserializedData);
    }
    return result;
}

template <typename DataType, typename Less>
void SerializableDataModel<DataType, Less>::deserializationComplete()
{
    dataModel_.publishPendingChanges();
}

} /* namespace Data */
