#pragma once

namespace Data {

/** Interface for reading data of given DataType */
template <typename DataType>
class DataReadIf
{
public:
    /** @return data value */
	virtual const DataType& get() const = 0;

    virtual ~DataReadIf() {}

protected:
    DataReadIf() {}

private:
    /** Non-copyable */
    DataReadIf(const DataReadIf&) = delete;
    DataReadIf& operator=(const DataReadIf&) = delete;
};

}
