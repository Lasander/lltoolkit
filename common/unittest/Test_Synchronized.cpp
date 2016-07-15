#include "../../common/Synchronized.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <iostream>

namespace Common {

namespace {

class DummyLock
{
public:
    void lock() {}
    void unlock() {}
};

// For printing output in cases the locks are hidden in the implementation and not accessible for the test
class PrintingLock
{
public:
    void lock() { std::cout << "lock" << std::endl; }
    void unlock() { std::cout << "unlock" << std::endl; }
};

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
    int getData() const { return data_; };

protected:
    MockData(int initialData) : data_(initialData) {}

    int data_;
};

class MockDataWithLock : public MockData
{
public:
    using MockData::MockData;

    void lock() { lock_.lock(); };
    void unlock() { lock_.unlock(); };

private:
    testing::NiceMock<MockLock> lock_;
};

class MockDataWithCustomLock : public MockData
{
public:
    using MockData::MockData;

    void lockMe() { lock_.lock(); };
    void unlockMe() { lock_.unlock(); };

private:
    testing::NiceMock<MockLock> lock_;
};

class CustomLock
{
public:
    void lock(MockDataWithCustomLock* data) { data->lockMe(); }
    void unlock(MockDataWithCustomLock* data) { data->unlockMe(); }
};

} // anonymous namespace

TEST(TestSynchronized, testSingleCall)
{
    testing::StrictMock<MockLock> lock;
    Synchronized<MockData, ExternalLock<MockLock>> data(lock, 66);

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
    Synchronized<MockData, InternalLock<testing::NiceMock<MockLock>>> data(66);

    EXPECT_EQ(66, data->getData());
    data->setData(88);
    EXPECT_EQ(88, data->getData());
}

TEST(TestSynchronized, testSelfLockType)
{
    Synchronized<MockDataWithLock, DataLock<MockDataWithLock>> data(66);

    EXPECT_EQ(66, data->getData());
    data->setData(88);
    EXPECT_EQ(88, data->getData());
}

TEST(TestSynchronized, testCustomLockType)
{
    Synchronized<MockDataWithCustomLock, CustomLock> data(66);

    EXPECT_EQ(66, data->getData());
    data->setData(88);
    EXPECT_EQ(88, data->getData());
}

TEST(TestSynchronized, testTransaction)
{
    testing::StrictMock<MockLock> lock;
    Synchronized<MockData, ExternalLock<MockLock>> data(lock, 55);

    testing::InSequence sequence;
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());

    auto transaction = data.makeTransaction();
    EXPECT_EQ(55, transaction->getData());
    transaction->setData(99);
    EXPECT_EQ(99, transaction->getData());
}

TEST(TestSynchronized, testConstTransaction)
{
    testing::StrictMock<MockLock> lock;
    const Synchronized<const MockData, ExternalLock<MockLock>> data(lock, 44);

    testing::InSequence sequence;
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());
    EXPECT_CALL(lock, unlock());

    auto transaction = data.makeTransaction();
    EXPECT_EQ(44, transaction->getData());
    EXPECT_EQ(44, data->getData());
}

TEST(TestSynchronized, testConstTransactionWithInternalLock)
{
    const Synchronized<const MockData, InternalLock<testing::NiceMock<MockLock>>> data(77);

    auto transaction = data.makeTransaction();
    EXPECT_EQ(77, transaction->getData());
    EXPECT_EQ(77, data->getData());
}

TEST(TestSynchronized, testSingleCallDuringTransaction)
{
    testing::StrictMock<MockLock> lock;
    Synchronized<MockData, ExternalLock<MockLock>> data(lock, 55);

    testing::InSequence sequence;
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, lock());
    EXPECT_CALL(lock, unlock());
    EXPECT_CALL(lock, unlock());

    auto transaction = data.makeTransaction();
    EXPECT_EQ(55, data->getData());
}

TEST(TestSynchronized, testCopy)
{
    Synchronized<MockData, InternalLock<DummyLock>> data(55);
    Synchronized<MockData, InternalLock<DummyLock>> copy(data);

    copy->setData(66);
    EXPECT_EQ(66, copy->getData());
    EXPECT_EQ(55, data->getData());
}

} // Common
