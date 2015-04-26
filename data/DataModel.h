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

template <typename T>
class DataModel : public Publisher<T>
{
public:
    DataModel();
    DataModel(const T& data);
    virtual ~DataModel();

    void set(const T& data);
    const T& get() const;

private:
    T data_;
    Publisher<T> dataPublisher_;
};

template <typename T>
DataModel<T>::DataModel()
 : data_{}
{
}

template <typename T>
DataModel<T>::DataModel(const T& data)
  : data_(data)
{
}

template <typename T>
DataModel<T>::~DataModel()
{
}

template <typename T>
void DataModel<T>::set(const T& data)
{
    data_ = data;
    Publisher<T>::notifySubscribers(data_);
}

template <typename T>
const T& DataModel<T>::get() const
{
    return data_;
}

}  // namespace Data

#endif /* DATA_DATAMODEL_H_ */
