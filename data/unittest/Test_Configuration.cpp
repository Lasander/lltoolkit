#include "../Configuration.hpp"
#include "../CascadingConfigurationRead.hpp"
#include "../SerializableIf.hpp"
#include "../Protobuf/ProtobufDataModel.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Test.pb.h" // Defines Number

using namespace testing;

namespace Data {
namespace {

class MockSerializable : public SerializableIf
{
public:
    MockSerializable()
    {
    	ON_CALL(*this, serialize(_)).WillByDefault(Return(true));
        ON_CALL(*this, deserialize(_)).WillByDefault(Return(true));
    }

    virtual ~MockSerializable() {}

    MOCK_CONST_METHOD1(serialize, bool(std::ostream& output));
    MOCK_METHOD1(deserialize, bool(std::istream& input));
    MOCK_METHOD0(deserializationComplete, void());
    
private:
	MockSerializable(const MockSerializable&) = delete;
	MockSerializable& operator=(const MockSerializable&) = delete;
};

//template <template <typename... ImplParams> class ImplType, typename DataType, typename... Params>
//class SerializableDataModelImpl : public SerializableDataModelIf<DataType>
//{
//public:
//  SerializableDataModelImpl();
//  virtual ~SerializableDataModelImpl();
//
//  /**
//   * @defgroup SerializableDataModelIf implementation
//   * @{
//   */
//    virtual void set(const DataType& data) override;
//    virtual const DataType& get() const override;
//  virtual Publisher<DataType>& publisher() override;
//
//    virtual bool serialize(std::ostream& output) const override;
//    virtual bool deserialize(std::istream& input) override;
//    virtual void deserializationComplete() override;
//    /**@}*/
//
//private:
//    SerializableDataModelImpl(const SerializableDataModelImpl&) = delete;
//    SerializableDataModelImpl& operator=(const SerializableDataModelImpl&) = delete;
//  ImplType<DataType, Params...> impl_;
//};


template <typename DataType, typename Less = std::less<DataType>>
class MockProtobufDataModel : public SerializableDataModelIf<DataType>
{
public:
    MockProtobufDataModel(const DataType& data)
    {
        impl_.set(data);

        ON_CALL(*this, set(_)).WillByDefault(Invoke(&impl_, &ImplType::set));
        ON_CALL(*this, get()).WillByDefault(Invoke(&impl_, &ImplType::get));
        ON_CALL(*this, publisher()).WillByDefault(Invoke(&impl_, &ImplType::publisher));
        ON_CALL(*this, deserialize(_)).WillByDefault(Invoke(&impl_, &ImplType::deserialize));
        ON_CALL(*this, serialize(_)).WillByDefault(Invoke(&impl_, &ImplType::serialize));
        ON_CALL(*this, deserializationComplete()).WillByDefault(Invoke(&impl_, &ImplType::deserializationComplete));
    }

    MockProtobufDataModel() : MockProtobufDataModel(DataType{}) {}

    virtual ~MockProtobufDataModel() {}

    MOCK_METHOD1_T(set, void(const DataType&));
    MOCK_CONST_METHOD0_T(get, const DataType&());
    MOCK_METHOD0_T(publisher, Publisher<DataType>&());

    MOCK_CONST_METHOD1(serialize, bool(std::ostream& output));
    MOCK_METHOD1(deserialize, bool(std::istream& input));
    MOCK_METHOD0(deserializationComplete, void());

    template <typename ValueType>
    void setValue(const ValueType& value)
    {
        DataType temp;
        temp.set_value(value);
        set(temp);
    }

private:
    MockProtobufDataModel(const MockProtobufDataModel&) = delete;
    MockProtobufDataModel& operator=(const MockProtobufDataModel&) = delete;

    using ImplType = Protobuf::ProtobufDataModel<DataType, Less>;

    ImplType impl_;
};

/** Comparison operation for Number type */
struct NumberLess
{
    bool operator()(const Number& x, const Number& y) const
    {
        return x.value() < y.value();
    }
};

} // anonymous namespace


TEST(Configuration, hasItem)
{
    Configuration c;
    NiceMock<MockSerializable> item;
    c.save("first", item);
    EXPECT_TRUE(c.hasItem("first"));
    EXPECT_TRUE(!c.hasItem("first_"));
}

TEST(Configuration, saveAndLoadNumber)
{
    InSequence sequence;

    using ModelType = NiceMock<MockProtobufDataModel<Number, NumberLess>>;
    ModelType number;
    number.setValue(1523423);
	
    Configuration c;
	EXPECT_CALL(number, serialize(_));
    EXPECT_TRUE(c.save("number", number));

    ModelType number2;
    EXPECT_CALL(number2, deserialize(_));
    EXPECT_TRUE(c.load("number", number2));

    EXPECT_EQ(number.get().value(), number2.get().value());
}

TEST(CascadingConfiguration, hasItem)
{
    InSequence sequence;

    using ModelType = NiceMock<MockProtobufDataModel<Number, NumberLess>>;
    ModelType number;

    Configuration parent;
    number.setValue(10000);
    EXPECT_TRUE(parent.save("number", number));
    number.setValue(20000);
    EXPECT_TRUE(parent.save("number2", number));

    Configuration conf;
    number.setValue(20001);
    EXPECT_TRUE(conf.save("number2", number));

    CascadingConfigurationRead cascade(conf, parent);
    EXPECT_TRUE(cascade.hasItem("number"));
    EXPECT_TRUE(cascade.hasItem("number2"));
}

TEST(CascadingConfiguration, saveAndLoad)
{
	InSequence sequence;

    using ModelType = NiceMock<MockProtobufDataModel<Number, NumberLess>>;
	ModelType number;

    Configuration parent;
    number.setValue(10000);
    EXPECT_TRUE(parent.save("number", number));
    number.setValue(20000);
    EXPECT_TRUE(parent.save("number2", number));

    Configuration conf;
    number.setValue(20001);
    EXPECT_TRUE(conf.save("number2", number));

    CascadingConfigurationRead cascade(conf, parent);

    ModelType result;
    EXPECT_TRUE(cascade.load("number", result));
    EXPECT_EQ(10000, result.get().value());

    EXPECT_TRUE(cascade.load("number2", result));
    EXPECT_EQ(20001, result.get().value());
}

} // Data
