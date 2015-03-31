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
    T data_;
    Publisher<T> dataPublisher_;

public:
    DataModel()
     : data_{}
    {
    }

    DataModel(const T& data)
      : data_(data)
    {
    }

    virtual ~DataModel()
    {
    }

    void set(const T& data)
    {
        data_ = data;
        Publisher<T>::notifySubscribers(data_);
    }

    const T& get() const
    {
        return data_;
    }

    T getCopy() const
    {
        return data_;
    }
};

}  // namespace Data

#endif /* DATA_DATAMODEL_H_ */
