#pragma once

namespace Data {

/** Serializes and deserializes given @p DataType */
template <typename DataType>
class SerializerIf
{
public:
    /**
     * Serialize @p data to @p output stream.
     * @return true if data was serialized.
     */
    virtual bool serialize(const DataType& data, std::ostream& output) const = 0;

    /**
     * Deserialize @p data from @p input stream.
     * @return true if data was deserialized.
     */
    virtual bool deserialize(DataType& data, std::istream& input) const = 0;

    virtual ~SerializerIf() {}
};

} /* namespace Data */
