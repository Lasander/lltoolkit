#include "test_util/LogHelpers.hpp"
#include "data/DataModel.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

namespace Data {
namespace {

/**
 * Test subscriber containing data of given type.
 * @tparam T data type
 */
template <typename T>
class Subscriber
{
public:
    MOCK_METHOD1_T(notifyReference, void(const T& data));
    MOCK_METHOD1_T(notifyValue, T(T data));
    MOCK_METHOD0_T(notifyEmpty, void());
};

template <class T>
class DataModelTest : public testing::Test
{
protected:
    DataModelTest() : data{}, pub{data.publisher()}, sub{} { currentSubscriber = &sub; }

    ~DataModelTest() { currentSubscriber = nullptr; }

    Common::ExpectNoErrorLogs noErrors;
    DataModel<T> data;
    Publisher<T>& pub;
    StrictMock<Subscriber<T>> sub;

public:
    /** Static link to current subscriber for free functions */
    static Subscriber<T>* currentSubscriber;
};
template <class T>
Subscriber<T>* DataModelTest<T>::currentSubscriber = nullptr;

template <class T>
void notifyFuncReference(const T& data)
{
    DataModelTest<T>::currentSubscriber->notifyReference(data);
}
template <class T>
void notifyFuncValue(T data)
{
    DataModelTest<T>::currentSubscriber->notifyValue(data);
}
template <class T>
int notifyFuncEmpty()
{
    DataModelTest<T>::currentSubscriber->notifyEmpty();
    return 99;
}

} // anonymous namespace

typedef DataModelTest<int> IntDataModelTest;
typedef DataModelTest<double> DoubleDataModelTest;
typedef DataModelTest<std::string> StringDataModelTest;

TEST_F(StringDataModelTest, defaultConstructor)
{
    DataModel<std::string> data;

    EXPECT_EQ(std::string(""), data.get());
}

TEST_F(StringDataModelTest, constructor)
{
    const std::string str = "Test string with some data to make it not so short.";
    DataModel<std::string> data(str);

    EXPECT_EQ(str, data.get());
}

TEST_F(StringDataModelTest, moveConstructor)
{
    std::string str = "Test string with some data to make it not so short.";
    DataModel<std::string> data(std::move(str));

    EXPECT_EQ(std::string("Test string with some data to make it not so short."), data.get());
}

TEST_F(IntDataModelTest, setAndGet)
{
    DataModel<int> data;
    data.set(2);

    EXPECT_EQ(2, data.get());
}

TEST_F(IntDataModelTest, methodNotifyReference)
{
    EXPECT_TRUE(pub.subscribe(sub, &Subscriber<int>::notifyReference));

    EXPECT_CALL(sub, notifyReference(3));
    data.set(3);

    EXPECT_TRUE(pub.unsubscribe(sub));
}

TEST_F(StringDataModelTest, methodNotifyValue)
{
    EXPECT_TRUE(pub.subscribe(sub, &Subscriber<std::string>::notifyValue));

    const char* testString = "test n0tification";
    EXPECT_CALL(sub, notifyValue(testString));
    data.set(testString);

    EXPECT_TRUE(pub.unsubscribe(sub));
}

TEST_F(StringDataModelTest, methodNotifyEmpty)
{
    EXPECT_TRUE(pub.subscribe(sub, &Subscriber<std::string>::notifyEmpty));

    EXPECT_CALL(sub, notifyEmpty());
    data.set("test data");

    EXPECT_TRUE(pub.unsubscribe(sub));
}

TEST_F(IntDataModelTest, functionNotifyReference)
{
    EXPECT_TRUE(pub.subscribe(sub, &notifyFuncReference<int>));

    EXPECT_CALL(sub, notifyReference(3));
    data.set(3);

    EXPECT_TRUE(pub.unsubscribe(sub));
}

TEST_F(StringDataModelTest, functionNotifyValue)
{
    EXPECT_TRUE(pub.subscribe(sub, &notifyFuncValue<std::string>));

    const char* testString = "test string";
    EXPECT_CALL(sub, notifyValue(testString));
    data.set(testString);

    EXPECT_TRUE(pub.unsubscribe(sub));
}

TEST_F(DoubleDataModelTest, functionNotifyEmpty)
{
    EXPECT_TRUE(pub.subscribe(sub, &notifyFuncEmpty<double>));

    EXPECT_CALL(sub, notifyEmpty());
    data.set(4.565);

    EXPECT_TRUE(pub.unsubscribe(sub));
}

TEST_F(IntDataModelTest, publishOnlyWhenRequested)
{
    EXPECT_TRUE(pub.subscribe(sub, &notifyFuncEmpty<int>));

    data.setInternal(5);
    data.setInternal(6);

    EXPECT_CALL(sub, notifyEmpty());
    data.publishPendingChanges();

    EXPECT_TRUE(pub.unsubscribe(sub));
}

TEST_F(IntDataModelTest, doubleSubscribe)
{
    EXPECT_TRUE(pub.subscribe(sub, &notifyFuncEmpty<int>));
    {
        Common::ExpectErrorLog error;
        EXPECT_FALSE(pub.subscribe(sub, &notifyFuncValue<int>));
    }

    EXPECT_CALL(sub, notifyEmpty());
    data.set(55);

    EXPECT_TRUE(pub.unsubscribe(sub));
    {
        Common::ExpectErrorLog error;
        EXPECT_FALSE(pub.unsubscribe(sub));
    }
}

} // namespace Data
