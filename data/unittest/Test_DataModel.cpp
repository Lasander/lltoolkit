#include "../DataModel.h"
#include "gtest/gtest.h"

namespace Data {

TEST(DataModelTest, setter)
{
    DataModel<int> data;
    data.set(2);
    
    EXPECT_EQ(2, data.get());
}

namespace {

/**
 * Test subscriber containing data of given type.
 *
 * @tparam T data type
 */
template<typename T>
class Subscriber
{
    /** Additional data used in notifications. */
    T additional_;

public:
    /** Data */
    T data_;

    /**
     * Construct Subscriber with given additional data.
     */
    Subscriber(const T& additional);

    /**
     * Receive notification of data change including data by const ref.
     * Sets data_ to "data + additional_".
     */
    void notifyReference(const T& data);
    /**
     * Receive notification of data change including data by value.
     * Sets data_ to "data + additional_".
     */
    void notifyValue(T data);
    /**
     * Receive notification of data change ignoring the data.
     * Sets data_ to "additional_".
     */
    void notifyEmpty();
};

template <typename T> Subscriber<T>::Subscriber(const T& additional)
  : additional_{additional}, data_{}
{
}
template <typename T> void Subscriber<T>::notifyReference(const T& data)
{
    data_ = data + additional_;
}
template <typename T> void Subscriber<T>::notifyValue(T data)
{
    data_ = data + additional_;
}
template <typename T> void Subscriber<T>::notifyEmpty()
{
    data_ = additional_;
}


/** Data modified by the notification functions */
int funcData = 0;

/**
 * Receive notification of data change including data by const ref.
 * Sets funcData to "data".
 */
void notifyFuncReference(const int& data)
{
    funcData = data;
}
/**
 * Receive notification of data change including data by value.
 * Sets funcData to "data * 2".
 */
void notifyFuncValue(int data)
{
    funcData = data * 2;
}
/**
 * Receive notification of data change ignoring the data.
 * Sets funcData to "99".
 */
void notifyFuncEmpty()
{
    funcData = 99;
}

} // anonymous namespace

TEST(DataModelTest, methodNotifyReference)
{
    DataModel<int> data;
    Subscriber<int> sub(2);
    EXPECT_TRUE(data.subscribe(sub, &Subscriber<int>::notifyReference));

    data.set(3);
    EXPECT_EQ(2 + 3, sub.data_);

    EXPECT_TRUE(data.unsubscribe(sub));
}

TEST(DataModelTest, methodNotifyValue)
{
    DataModel<double> data;
    Subscriber<double> sub(5.6);
    EXPECT_TRUE(data.subscribe(sub, &Subscriber<double>::notifyValue));

    data.set(4.1);
    EXPECT_DOUBLE_EQ(5.6 + 4.1, sub.data_);

    EXPECT_TRUE(data.unsubscribe(sub));
}

TEST(DataModelTest, methodNotifyEmpty)
{
    DataModel<int> data;
    Subscriber<std::string> sub("additional");
    EXPECT_TRUE(data.subscribe(sub, &Subscriber<std::string>::notifyEmpty));

    data.set(4);
    EXPECT_STREQ("additional", sub.data_.c_str());

    EXPECT_TRUE(data.unsubscribe(sub));
}

TEST(DataModelTest, functionNotifyReference)
{
    DataModel<int> data;
    Subscriber<int> sub(1);
    EXPECT_TRUE(data.subscribe(sub, &notifyFuncReference));

    data.set(3);
    EXPECT_EQ(3, funcData);

    EXPECT_TRUE(data.unsubscribe(sub));
}

TEST(DataModelTest, functionNotifyValue)
{
    DataModel<int> data;
    Subscriber<int> sub(1);
    EXPECT_TRUE(data.subscribe(sub, &notifyFuncValue));

    data.set(4);
    EXPECT_EQ(8, funcData);

    EXPECT_TRUE(data.unsubscribe(sub));
}

TEST(DataModelTest, functionNotifyEmpty)
{
    DataModel<int> data;
    Subscriber<int> sub(1);
    EXPECT_TRUE(data.subscribe(sub, &notifyFuncEmpty));

    data.set(4);
    EXPECT_EQ(99, funcData);

    EXPECT_TRUE(data.unsubscribe(sub));
}

TEST(DataModelTest, doubleSubscribe)
{
    DataModel<int> data;
    Subscriber<int> sub(1);
    EXPECT_TRUE(data.subscribe(sub, &notifyFuncEmpty));
    EXPECT_FALSE(data.subscribe(sub, &notifyFuncValue));

    data.set(55);
    EXPECT_EQ(99, funcData);

    EXPECT_TRUE(data.unsubscribe(sub));
    EXPECT_FALSE(data.unsubscribe(sub));
}

} // Data

int main(int argc, char **argv)
{
    freopen("/dev/null", "w", stderr);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
