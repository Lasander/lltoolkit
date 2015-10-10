/**
 * DataModel.hpp
 *
 *  Created on: Mar 29, 2015
 *      Author: lasse
 */

#ifndef DATA_DATAMODEL_HPP_
#define DATA_DATAMODEL_HPP_

#include "Publisher.hpp"
#include "DataModelIf.hpp"
#include <utility>
#include <functional>

namespace Data {

/**
 * Implementation of a data model interface. Contains data which can be set and get.
 * Additionally one can register to receive synchronous notification of the
 * model value change using the attached publisher.
 *
 * Not thread-safe.
 *
 * @tparam DataType Data type
 */
template <typename DataType, typename Less = std::less<DataType>>
class DataModel : public DataModelIf<DataType>
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
    virtual void set(const DataType& data);

    /**
     * @return Model value
     */
    virtual const DataType& get() const;

    /** @return publisher */
	virtual Publisher<DataType>& publisher();

    /** Set internal model value to @p data, but do not publish the change. */
    void setInternal(const DataType& data);

    /** Publish any pending (unpublished) changes to the model. If there are no pending changes, does nothing. */
    void publishPendingChanges();

private:
    /** Model data */
    DataType data_;

    /** Publisher */
    Publisher<DataType> publisher_;

    /** True if internal state has been changed since previous publish. @see setInternal */
    bool hasUnpublishedChanges_;
};

template <typename DataType, typename Less>
DataModel<DataType, Less>::DataModel() :
	data_{},
	publisher_{},
	hasUnpublishedChanges_{false}
{
}

template <typename DataType, typename Less>
DataModel<DataType, Less>::DataModel(const DataType& data) :
	data_(data),
	publisher_{},
	hasUnpublishedChanges_{false}
{
}

template <typename DataType, typename Less>
DataModel<DataType, Less>::DataModel(DataType&& data) :
	data_{std::forward<DataType>(data)},
	publisher_{},
	hasUnpublishedChanges_{false}
{
}

template <typename DataType, typename Less>
DataModel<DataType, Less>::~DataModel()
{
}

template <typename DataType, typename Less>
void DataModel<DataType, Less>::set(const DataType& data)
{
	setInternal(data);
	publishPendingChanges();
}

template <typename DataType, typename Less>
const DataType& DataModel<DataType, Less>::get() const
{
    return data_;
}

template <typename DataType, typename Less>
Publisher<DataType>& DataModel<DataType, Less>::publisher()
{
    return publisher_;
}

template <typename DataType, typename Less>
void DataModel<DataType, Less>::setInternal(const DataType& data)
{
	const Less less{};
	const bool differ = less(data_, data) || less(data, data_);

    if (differ)
    {
        data_ = data;
        hasUnpublishedChanges_ = true;
    }
}

template <typename DataType, typename Less>
void DataModel<DataType, Less>::publishPendingChanges()
{
	if (hasUnpublishedChanges_)
	{
		hasUnpublishedChanges_ = false;
		publisher_.notifySubscribers(data_);
	}
}

}  // namespace Data

#endif /* DATA_DATAMODEL_HPP_ */
