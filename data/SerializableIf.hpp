#ifndef DATA_SERIALIZABLEIF_H_
#define DATA_SERIALIZABLEIF_H_

#include <iosfwd>

namespace Data {

/**
 * Interface for serializable item
 */
class SerializableIf
{
public:
	/** Save item to @p output stream */
    virtual bool serialize(std::ostream& output) const = 0;

    /** Load item from @p input stream */
    virtual bool deserialize(std::istream& input) = 0;

    /** Notification that a transactions involving this object has been completed. */
    virtual void deserializationComplete() = 0;

    virtual ~SerializableIf() {}
};

} // Data

#endif
