#pragma once

#include "DataIf.hpp"
#include <utility>

namespace Data {

/** Implementation of data read/write interface */
template <typename DataType>
class Data : public DataIf<DataType>
{
public:
    /** Construct DataModel with default data */
    Data();

    /**
     * Construct DataModel by copying given initial data
     *
     * @param data Initial data
     */
    Data(const DataType& data);

    /**
     * Construct DataModel by moving given initial data
     *
     * @param data Initial data
     */
    Data(DataType&& data);

    /** Virtual dtor */
    ~Data() override;

    /**
     * Set model value
     *
     * @param data New value
     */
    void set(const DataType& data) override;

    /**
     * @return Model value
     */
    const DataType& get() const override;

private:
    /** Model data */
    DataType data_;
};

template <typename DataType>
Data<DataType>::Data() : data_{}
{
}

template <typename DataType>
Data<DataType>::Data(const DataType& data) : data_(data)
{
}

template <typename DataType>
Data<DataType>::Data(DataType&& data) : data_(std::forward<DataType>(data))
{
}

template <typename DataType>
Data<DataType>::~Data()
{
}

template <typename DataType>
void Data<DataType>::set(const DataType& data)
{
    data_ = data;
}

template <typename DataType>
const DataType& Data<DataType>::get() const
{
    return data_;
}

} // namespace Data
