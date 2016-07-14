#include "../../common/Synchronized.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace Common {

namespace {

class MockLock
{
public:
    MOCK_METHOD0(lock, void());
    MOCK_METHOD0(unlock, void());
};

class MockData
{
public:
    void setData(int data) { data_ = data;}
    int getData() { return data_; };

protected:
    MockData(int initialData) : data_(initialData) {}

    int data_;
};

class MockDataWithLock : public MockData
{
public:
    using MockData::MockData;

protected:
    void lock() { lock_.lock(); };
    void unlock() { lock_.unlock(); };

private:
    MockLock lock_;
};

} // anonymous namespace

TEST(TestSynchronized, testSingleCall)
{
    testing::StrictMock<MockLock> lock;
    ExternalLock<MockLock> lockPolicy(lock);
    Synchronized<MockData, ExternalLock<MockLock>> data(lockPolicy, 66);

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
    Synchronized<MockData> data(66);

    EXPECT_EQ(66, data->getData());
    data->setData(88);
    EXPECT_EQ(88, data->getData());
}

TEST(TestSynchronized, testInternalLockType)
{
    Synchronized<MockData, InternalLock<MockLock>> data(66);

    EXPECT_EQ(66, data->getData());
    data->setData(88);
    EXPECT_EQ(88, data->getData());
}

TEST(TestSynchronized, testSelfLockType)
{
    Synchronized<MockDataWithLock, DataLock> data(66);

    EXPECT_EQ(66, data->getData());
    data->setData(88);
    EXPECT_EQ(88, data->getData());
}

TEST(TestSynchronized, testTransaction)
{
    testing::StrictMock<MockLock> lock;
    ExternalLock<MockLock> lockPolicy(lock);
    Synchronized<MockData, ExternalLock<MockLock>> data(lockPolicy, 55);

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
    ExternalLock<MockLock> lockPolicy(lock);
    Synchronized<MockData, ExternalLock<MockLock>> data(lockPolicy, 55);

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
    ExternalLock<MockLock> lockPolicy(lock);
    Synchronized<MockData, ExternalLock<MockLock>> data(lockPolicy, 55);
    Synchronized<MockData, ExternalLock<MockLock>> copy(data);

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
