#include "../Configuration.hpp"
#include "../CascadingConfigurationRead.hpp"
#include "../DataModel.hpp"
#include "../SerializableIf.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Test.pb.h"

using namespace testing;

namespace Data {
namespace {

class MockItem : public SerializableIf
{
public:
    MockItem()
    {
    	ON_CALL(*this, serialize(_)).WillByDefault(Return(true));
        ON_CALL(*this, deserialize(_)).WillByDefault(Return(true));
    }

    virtual ~MockItem() {}

    MOCK_CONST_METHOD1(serialize, bool(std::ostream& output));
    MOCK_METHOD1(deserialize, bool(std::istream& input));
    MOCK_METHOD0(deserializationComplete, void());
    
private:
	MockItem(const MockItem&) = delete;
	MockItem& operator=(const MockItem&) = delete;
};

template <typename DataType>
class ProtobufItem : public SerializableIf
{
public:
    ProtobufItem(const DataType& data) :
    	data_(data)
    {
		ON_CALL(*this, serialize(_)).WillByDefault(Invoke(this, &ProtobufItem<DataType>::serializeImpl));
    	ON_CALL(*this, deserialize(_)).WillByDefault(Invoke(this, &ProtobufItem<DataType>::deserializeImpl));
	}

    ProtobufItem() : ProtobufItem(DataType{}) {}

    virtual ~ProtobufItem() {}

    MOCK_CONST_METHOD1(serialize, bool(std::ostream& output));
    MOCK_METHOD1(deserialize, bool(std::istream& input));
    MOCK_METHOD0(deserializationComplete, void());
    
    DataType& item()
    {
        return data_;
    }
    
private:
	ProtobufItem(const ProtobufItem&) = delete;
	ProtobufItem& operator=(const ProtobufItem&) = delete;

    bool serializeImpl(std::ostream& output) const
    {
        return data_.SerializeToOstream(&output);
    }
    bool deserializeImpl(std::istream& input)
    {
        return data_.ParseFromIstream(&input);
    }
    
    DataType data_;
};

} // anonymous namespace


TEST(Configuration, hasItem)
{
    Configuration c;
    NiceMock<MockItem> item;
    c.save("first", item);
    EXPECT_TRUE(c.hasItem("first"));
    EXPECT_TRUE(!c.hasItem("first_"));
}

TEST(Configuration, saveAndLoad)
{
	InSequence sequence;

    Number n;
    n.set_value(1523423);
    StrictMock<ProtobufItem<Number>> number(n);
	
    Configuration c;
	EXPECT_CALL(number, serialize(_));
    EXPECT_TRUE(c.save("number", number));

    StrictMock<ProtobufItem<Number>> number2;
	EXPECT_CALL(number2, deserialize(_));
    EXPECT_TRUE(c.load("number", number2));

    EXPECT_EQ(number.item().value(), number2.item().value());
}

TEST(CascadingConfiguration, hasItem)
{
	InSequence sequence;

    NiceMock<ProtobufItem<Number>> number;

    Configuration parent;
    number.item().set_value(10000);
    EXPECT_TRUE(parent.save("number", number));
    number.item().set_value(20000);
    EXPECT_TRUE(parent.save("number2", number));

    Configuration conf;
    number.item().set_value(20001);
    EXPECT_TRUE(conf.save("number2", number));

    CascadingConfigurationRead cascade(conf, parent);
    EXPECT_TRUE(cascade.hasItem("number"));
    EXPECT_TRUE(cascade.hasItem("number2"));
}

TEST(CascadingConfiguration, saveAndLoad)
{
	InSequence sequence;

    NiceMock<ProtobufItem<Number>> number;

    Configuration parent;
    number.item().set_value(10000);
    EXPECT_TRUE(parent.save("number", number));
    number.item().set_value(20000);
    EXPECT_TRUE(parent.save("number2", number));

    Configuration conf;
    number.item().set_value(20001);
    EXPECT_TRUE(conf.save("number2", number));

    CascadingConfigurationRead cascade(conf, parent);

    NiceMock<ProtobufItem<Number>> result;
    EXPECT_TRUE(cascade.load("number", result));
    EXPECT_EQ(10000, result.item().value());

    EXPECT_TRUE(cascade.load("number2", result));
    EXPECT_EQ(20001, result.item().value());
}

namespace {

template <typename DataType, typename Less = std::less<DataType>>
class DataModelItem :
	public SerializableIf,
	private DataModelIf<DataType>
{
public:
    /** Construct DataModel with default data */
	DataModelItem() {}

    /** Virtual dtor */
    virtual ~DataModelItem() {}

    virtual void set(const DataType& data) override
    {
    	dataModel_.set(data);
    }

    virtual const DataType& get() const override
    {
    	return dataModel_.get();
    }

	virtual Publisher<DataType>& publisher()  override
	{
		return dataModel_.publisher();
	}

    bool serialize(std::ostream& output) const override
    {
        return get().SerializeToOstream(&output);
    }

    bool deserialize(std::istream& input) override
    {
    	DataType deserializedData{};
    	bool result = deserializedData.ParseFromIstream(&input);
        if (result)
        {
        	dataModel_.setInternal(deserializedData);
        }
        return result;
    }

    virtual void deserializationComplete() override
    {
    	dataModel_.publishPendingChanges();
    }

private:
    DataModelItem(const DataModelItem&) = delete;
    DataModelItem& operator=(const DataModelItem&) = delete;

    DataModel<DataType, Less> dataModel_;
};

struct NumberLess
{
	bool operator()(const Number& x, const Number& y) const
	{
		return x.value() < y.value();
	}
};

} // anonymous namespace

TEST(DataModelItem, saveAndLoad)
{
	InSequence sequence;

    NiceMock<DataModelItem<Number, NumberLess>> number;

    Configuration parent;
    Number temp;
    temp.set_value(10000);
    number.set(temp);
    EXPECT_TRUE(parent.save("number", number));
    temp.set_value(20000);
    number.set(temp);
    EXPECT_TRUE(parent.save("number2", number));

    Configuration conf;
    temp.set_value(20001);
    number.set(temp);
    EXPECT_TRUE(conf.save("number2", number));

    CascadingConfigurationRead cascade(conf, parent);

    NiceMock<DataModelItem<Number, NumberLess>> result;
    EXPECT_TRUE(cascade.load("number", result));
    EXPECT_EQ(10000, result.get().value());

    EXPECT_TRUE(cascade.load("number2", result));
    EXPECT_EQ(20001, result.get().value());
}

} // Data
