#pragma once

namespace Data {

/** Interface for writing data of given DataType */
template <typename DataType>
class DataWriteIf
{
public:
    /** Write @p data */
    virtual void set(const DataType& data) = 0;

    virtual ~DataWriteIf() {}

protected:
    DataWriteIf() {}

private:
    /** Non-copyable */
    DataWriteIf(const DataWriteIf&) = delete;
    DataWriteIf& operator=(const DataWriteIf&) = delete;
};

} // namespace Data
