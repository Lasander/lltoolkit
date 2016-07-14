#include "../../common/Synchronized.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace Common {

namespace {

class MockData
{
public:
    void setData(int data) { data_ = data;}
    int getData() { return data_; };

private:
    // Allow only synchronized objects
    template <typename Data, typename Lock> friend class Common::Synchronized;
    MockData(int initialData) : data_(initialData) {}

    int data_;
};

class MockLock
{
public:
    MOCK_METHOD0(lock, void());
    MOCK_METHOD0(unlock, void());
};

} // anonymous namespace

TEST(TestSynchronized, testSingleCall)
{
    testing::StrictMock<MockLock> lock;
    Synchronized<MockData, MockLock> data(lock, 66);

    testing::InSequence sequence;
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());

    EXPECT_EQ(66, data->getData());
    data->setData(88);
    EXPECT_EQ(88, data->getData());

    testing::Mock::VerifyAndClearExpectations(&lock);
}

TEST(TestSynchronized, testDefaultLockType)
{
    std::mutex mutex;
    Synchronized<MockData> data(mutex, 66);

    EXPECT_EQ(66, data->getData());
    data->setData(88);
    EXPECT_EQ(88, data->getData());
}

TEST(TestSynchronized, testTransaction)
{
    testing::StrictMock<MockLock> lock;
    Synchronized<MockData, MockLock> data(lock, 55);

    testing::InSequence sequence;
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());

    auto transaction = data.makeTransaction();
    EXPECT_EQ(55, transaction->getData());
    transaction->setData(99);
    EXPECT_EQ(99, transaction->getData());
}

TEST(TestSynchronized, testSingleCallDuringTransaction)
{
    testing::StrictMock<MockLock> lock;
    Synchronized<MockData, MockLock> data(lock, 55);

    testing::InSequence sequence;
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());
    EXPECT_CALL(lock, unlock());

    auto transaction = data.makeTransaction();
    EXPECT_EQ(55, transaction->getData());
    data->setData(99);
}

TEST(TestSynchronized, testCopy)
{
    testing::StrictMock<MockLock> lock;
    Synchronized<MockData, MockLock> data(lock, 55);
    Synchronized<MockData, MockLock> copy(data);

    testing::InSequence sequence;
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());

    copy->setData(66);
    EXPECT_EQ(66, copy->getData());
    EXPECT_EQ(55, data->getData());

}

} // Common