/**
 * DataModel.hpp
 *
 *  Created on: Mar 29, 2015
 *      Author: lasse
 */

#ifndef DATA_DATAMODEL_H_
#define DATA_DATAMODEL_H_

#include "Publisher.h"

namespace Data {

/**
 * Data model of given type. Model contains data which can be set and get.
 * Additionally one can register to receive synchronous notification of the
 * model value change.
 *
 * @tparam DataType Data type
 */
template <typename DataType>
class DataModel : public Publisher<DataType>
{
public:
    /** Construct DataModel with default data */
    DataModel();

    /**
     * Construct DataModel by copying given initial data
     *
     * @param data Initial data
     */
    DataModel(const DataType& data);

    /**
     * Construct DataModel by moving given initial data
     *
     * @param data Initial data
     */
    DataModel(DataType&& data);

    /** Virtual dtor */
    virtual ~DataModel();

    /**
     * Set model value
     *
     * @param data New value
     */
    void set(const DataType& data);

    /**
     * @return Model value
     */
    const DataType& get() const;

private:
    /** Prevent copy, move and assignment */
    DataModel(const DataModel&);
    DataModel(DataModel&&);
    DataModel& operator=(const DataModel&);

    /** Model data */
    DataType data_;
};

template <typename DataType>
DataModel<DataType>::DataModel()
 : data_{}
{
}

template <typename DataType>
DataModel<DataType>::DataModel(const DataType& data)
  : data_(data)
{
}

template <typename DataType>
DataModel<DataType>::DataModel(DataType&& data)
  : data_(data)
{
}

template <typename DataType>
DataModel<DataType>::~DataModel()
{
}

template <typename DataType>
void DataModel<DataType>::set(const DataType& data)
{
    if (data_ != data)
    {
        data_ = data;
        Publisher<DataType>::notifySubscribers(data_);
    }
}

template <typename DataType>
const DataType& DataModel<DataType>::get() const
{
    return data_;
}

}  // namespace Data

#endif /* DATA_DATAMODEL_H_ */
