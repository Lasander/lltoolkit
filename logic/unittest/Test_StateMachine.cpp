#include "../../common/unittest/LogHelpers.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>
#include <iostream>
#include <map>
#include "../StateMachine.hpp"

using namespace testing;

namespace Logic {
namespace {

class MockMachine
{
public:
    MockMachine()
      : impl_(A21)
    {
        impl_.onEntry(A).invoke(*this, &MockMachine::A_Entry);
        impl_.onExit(A).invoke(*this, &MockMachine::A_Exit);
        impl_.onEntry(A1).invoke(*this, &MockMachine::A1_Entry);
        impl_.onExit(A1).invoke(*this, &MockMachine::A1_Exit);
        impl_.onEntry(A2).invoke(*this, &MockMachine::A2_Entry);
        impl_.onExit(A2).invoke(*this, &MockMachine::A2_Exit);
        impl_.onEntry(A21).invoke(*this, &MockMachine::A21_Entry);
        impl_.onExit(A21).invoke(*this, &MockMachine::A21_Exit);
        impl_.onEntry(B).invoke(*this, &MockMachine::B_Entry);
        impl_.onExit(B).invoke(*this, &MockMachine::B_Exit);
        impl_.onEntry(B1).invoke(*this, &MockMachine::B1_Entry);
        impl_.onExit(B1).invoke(*this, &MockMachine::B1_Exit);
        impl_.onEntry(C).invoke(*this, &MockMachine::C_Entry);
        impl_.onExit(C).invoke(*this, &MockMachine::C_Exit);
        impl_.onEntry(C1).invoke(*this, &MockMachine::C1_Entry);
        impl_.onExit(C1).invoke(*this, &MockMachine::C1_Exit);

        // Define state hierarchy
        impl_.setParent(A, {A1, A2});
        impl_.setParent(A2, A21);
        impl_.setParent(B, B1);
        impl_.setParent(C, C1);


        using MemberFn = void(MockMachine::*)();
        MemberFn to_Events[] =
        {
            &MockMachine::to_A,
            &MockMachine::to_A1,
            &MockMachine::to_A2,
            &MockMachine::to_A21,
            &MockMachine::to_B,
            &MockMachine::to_B1
        };

        // Define all transitions between states from A to B1
        for (int i = A; i <= B1 ; ++i)
        {
            State from = static_cast<State>(i);
            for (int j = A; j <= B1 ; ++j)
            {
                State to = static_cast<State>(j);
                impl_.onTransition(from, to, to_Events[j]).invoke(*this, &MockMachine::AB_action);
            }
            // Internal action
            impl_.onTransition(from, &MockMachine::to_self).invoke(*this, &MockMachine::internalAction);
        }

        impl_.onTransition(B1, C1, &MockMachine::from_B1_to_C1_with_move).invoke(*this, &MockMachine::C1_action_with_move_);
        impl_.onTransition(B1, C1, &MockMachine::from_B1_to_C1).invoke(*this, &MockMachine::C1_action);

        impl_.onTransition(A1, C1, &MockMachine::from_A1_to_C1_with_condition).when(*this, &MockMachine::A1_C1_condition);
        impl_.onTransition(A1, C1, &MockMachine::from_A1_to_C1_with_condition).when(*this, &MockMachine::A1_C1_condition_2);
        impl_.onTransition(A, C1, &MockMachine::from_A1_to_C1_with_condition).when(*this, &MockMachine::A_C1_condition);
        impl_.onTransition(C1, &MockMachine::to_self).invoke(*this, &MockMachine::internalAction);
    }

    MOCK_METHOD0(A_Entry, void());
    MOCK_METHOD0(A_Exit, void());
    MOCK_METHOD0(A1_Entry, void());
    MOCK_METHOD0(A1_Exit, void());
    MOCK_METHOD0(A2_Entry, void());
    MOCK_METHOD0(A2_Exit, void());
    MOCK_METHOD0(A21_Entry, void());
    MOCK_METHOD0(A21_Exit, void());
    MOCK_METHOD0(B_Entry, void());
    MOCK_METHOD0(B_Exit, void());
    MOCK_METHOD0(B1_Entry, void());
    MOCK_METHOD0(B1_Exit, void());
    MOCK_METHOD0(C_Entry, void());
    MOCK_METHOD0(C_Exit, void());
    MOCK_METHOD0(C1_Entry, void());
    MOCK_METHOD0(C1_Exit, void());
    MOCK_METHOD0(internalAction, void());

    MOCK_METHOD0(AB_action, void());

    void C1_action_with_move_(std::unique_ptr<int> i) { C1_action_with_move(i); }
    MOCK_METHOD1(C1_action_with_move, void(const std::unique_ptr<int>& i));
    MOCK_METHOD1(C1_action, void(int i));

    MOCK_CONST_METHOD0(A1_C1_condition, bool());
    MOCK_CONST_METHOD0(A1_C1_condition_2, bool());
    MOCK_CONST_METHOD0(A_C1_condition, bool());

    enum State
    {
        A,
        A1,
        A2,
        A21,
        B,
        B1,
        C,
        C1
    };

    void to_A() { impl_(_to_A); }
    void to_A1() { impl_(&MockMachine::to_A1); }
    void to_A2() { impl_(&MockMachine::to_A2); }
    void to_A21() { impl_(&MockMachine::to_A21); }
    void to_B() { impl_(&MockMachine::to_B); }
    void to_B1() { impl_(&MockMachine::to_B1); }
    void to_self() { impl_(&MockMachine::to_self); }

    void from_A1_to_C1_with_condition() { impl_(&MockMachine::from_A1_to_C1_with_condition); }

    void from_B1_to_C1(int i) { impl_(&MockMachine::from_B1_to_C1, i); }
    void from_B1_to_C1_with_move(std::unique_ptr<int> i) { impl_(&MockMachine::from_B1_to_C1_with_move, std::move(i)); }


    void enterInitialState() { impl_.enterInitialState(); }
    State getState() const { return impl_.getState(); }

    static constexpr const auto _to_A = &MockMachine::to_A;

private:
    StateMachine<MockMachine, State> impl_;
};

class MockMachineTest : public Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    void verifyExpectations()
    {
        Mock::VerifyAndClearExpectations(&m);
    }

    StrictMock<MockMachine> m;
};

class InitializedMockMachineTest : public MockMachineTest
{
protected:
    void SetUp() override
    {
        MockMachineTest::SetUp();

        // Expect entry to initial state
        InSequence sequence;
        EXPECT_CALL(m, A_Entry());
        EXPECT_CALL(m, A2_Entry());
        EXPECT_CALL(m, A21_Entry());

        // When
        m.enterInitialState();

        // Then
        verifyExpectations();
        EXPECT_EQ(MockMachine::A21, m.getState());
    }

    void inA1()
    {
        // Given
        EXPECT_EQ(MockMachine::A21, m.getState());

        // Expect
        EXPECT_CALL(m, A21_Exit());
        EXPECT_CALL(m, A2_Exit());
        EXPECT_CALL(m, AB_action());
        EXPECT_CALL(m, A1_Entry());

        // When
        m.to_A1();

        // Then
        verifyExpectations();
        EXPECT_EQ(MockMachine::A1, m.getState());
    }

    void inB1()
    {
        // Given
        EXPECT_EQ(MockMachine::A21, m.getState());

        // Expect
        EXPECT_CALL(m, A21_Exit());
        EXPECT_CALL(m, A2_Exit());
        EXPECT_CALL(m, A_Exit());
        EXPECT_CALL(m, AB_action());
        EXPECT_CALL(m, B_Entry());
        EXPECT_CALL(m, B1_Entry());

        // When
        m.to_B1();

        // Then
        verifyExpectations();
        EXPECT_EQ(MockMachine::B1, m.getState());
    }
};

} // anonymous namespace

TEST_F(MockMachineTest, initialEntry)
{
    // Expect entry to initial state
    InSequence sequence;
    EXPECT_CALL(m, A_Entry());
    EXPECT_CALL(m, A2_Entry());
    EXPECT_CALL(m, A21_Entry());

    // When
    m.enterInitialState();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::A21, m.getState());
}

TEST_F(MockMachineTest, initialEntryWithInternalAction)
{
    // Expect entry to initial state
    InSequence sequence;
    EXPECT_CALL(m, A_Entry());
    EXPECT_CALL(m, A2_Entry());
    EXPECT_CALL(m, A21_Entry());
    // And internal action
    EXPECT_CALL(m, internalAction());

    // When internal action as the first event
    m.to_self();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::A21, m.getState());
}

TEST_F(MockMachineTest, initialEntryWithTransitionEvent)
{
    // Expect entry to initial state
    InSequence sequence;
    EXPECT_CALL(m, A_Entry());
    EXPECT_CALL(m, A2_Entry());
    EXPECT_CALL(m, A21_Entry());
    // And transition
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, A2_Exit());
    EXPECT_CALL(m, A_Exit());
    EXPECT_CALL(m, AB_action());
    EXPECT_CALL(m, B_Entry());
    EXPECT_CALL(m, B1_Entry());

    // When handling first event
    m.to_B1();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::B1, m.getState());
}

TEST_F(InitializedMockMachineTest, simpleTransition)
{
    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, A2_Exit());
    EXPECT_CALL(m, A_Exit());
    EXPECT_CALL(m, AB_action());
    EXPECT_CALL(m, B_Entry());
    EXPECT_CALL(m, B1_Entry());

    // When handling event
    m.to_B1();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::B1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionToSelf)
{
    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, AB_action());
    EXPECT_CALL(m, A21_Entry());

    // When
    m.to_A21();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::A21, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionToParent)
{
    // Expect
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, AB_action());

    // When
    m.to_A2();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::A2, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionToGrandParent)
{
    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, A2_Exit());
    EXPECT_CALL(m, AB_action());

    // When
    m.to_A();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::A, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionToSibling)
{
    // Given in A2
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, AB_action());
    m.to_A2();
    verifyExpectations();

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A2_Exit());
    EXPECT_CALL(m, AB_action());
    EXPECT_CALL(m, A1_Entry());

    // When transit to A1
    m.to_A1();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::A1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithAction)
{
    // Given
    inB1();
    const int data = 5;

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, B1_Exit());
    EXPECT_CALL(m, B_Exit());
    EXPECT_CALL(m, C1_action(data));
    EXPECT_CALL(m, C_Entry());
    EXPECT_CALL(m, C1_Entry());

    // When
    m.from_B1_to_C1(data);

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::C1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithActionWithMove)
{
    // Given
    inB1();
    const int data = 5;

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, B1_Exit());
    EXPECT_CALL(m, B_Exit());
    EXPECT_CALL(m, C1_action_with_move(_)).WillOnce(Invoke([data](const std::unique_ptr<int>& i){ EXPECT_EQ(data, *i); }));
    EXPECT_CALL(m, C_Entry());
    EXPECT_CALL(m, C1_Entry());

    // When
    m.from_B1_to_C1_with_move(std::make_unique<int>(data));

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::C1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithTrueCondition)
{
    // Given
    inA1();

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A1_C1_condition()).WillOnce(Return(true));
    EXPECT_CALL(m, A1_Exit());
    EXPECT_CALL(m, A_Exit());
    EXPECT_CALL(m, C_Entry());
    EXPECT_CALL(m, C1_Entry());

    // When
    m.from_A1_to_C1_with_condition();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::C1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithSecondaryCondition)
{
    // Given
    inA1();

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A1_C1_condition()).WillOnce(Return(false));
    EXPECT_CALL(m, A1_C1_condition_2()).WillOnce(Return(true));
    EXPECT_CALL(m, A1_Exit());
    EXPECT_CALL(m, A_Exit());
    EXPECT_CALL(m, C_Entry());
    EXPECT_CALL(m, C1_Entry());

    // When
    m.from_A1_to_C1_with_condition();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::C1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithParentFallback)
{
    // Given
    inA1();

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A1_C1_condition()).WillOnce(Return(false));
    EXPECT_CALL(m, A1_C1_condition_2()).WillOnce(Return(false));
    EXPECT_CALL(m, A_C1_condition()).WillOnce(Return(true));
    EXPECT_CALL(m, A1_Exit());
    EXPECT_CALL(m, A_Exit());
    EXPECT_CALL(m, C_Entry());
    EXPECT_CALL(m, C1_Entry());

    // When
    m.from_A1_to_C1_with_condition();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::C1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithParentFallbackWithFalseCondition)
{
    // Given
    inA1();

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A1_C1_condition()).WillOnce(Return(false));
    EXPECT_CALL(m, A1_C1_condition_2()).WillOnce(Return(false));
    EXPECT_CALL(m, A_C1_condition()).WillOnce(Return(false));

    // When
    {
        Common::ExpectErrorLog errors; // Expect unhandled event error
        m.from_A1_to_C1_with_condition();
    }

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::A1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithRecursiveTransitionEvent)
{
    // Given
    const int data = 6;

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, A2_Exit());
    EXPECT_CALL(m, A_Exit());
    EXPECT_CALL(m, AB_action()).WillOnce(Invoke([this, data] { m.from_B1_to_C1(data); }));
    EXPECT_CALL(m, B_Entry());
    EXPECT_CALL(m, B1_Entry());
    EXPECT_CALL(m, B1_Exit());
    EXPECT_CALL(m, B_Exit());
    EXPECT_CALL(m, C1_action(data));
    EXPECT_CALL(m, C_Entry());
    EXPECT_CALL(m, C1_Entry());

    // When
    m.to_B1();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::C1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithRecursiveInternalAction)
{
    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, A2_Exit());
    EXPECT_CALL(m, A_Exit());
    EXPECT_CALL(m, AB_action()).WillOnce(Invoke([this] { m.to_self(); }));
    EXPECT_CALL(m, B_Entry());
    EXPECT_CALL(m, B1_Entry());
    EXPECT_CALL(m, internalAction());

    // When
    m.to_B1();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::B1, m.getState());
}

TEST_F(InitializedMockMachineTest, transitionWithMultipleRecursiveEvents)
{
    // Given
    const int data = 6;

    // Expect
    InSequence sequence;
    EXPECT_CALL(m, A21_Exit());
    EXPECT_CALL(m, A2_Exit());
    EXPECT_CALL(m, A_Exit());
    EXPECT_CALL(m, AB_action()).WillOnce(Invoke([this, data] { m.from_B1_to_C1(data); m.to_self(); }));
    EXPECT_CALL(m, B_Entry());
    EXPECT_CALL(m, B1_Entry());
    EXPECT_CALL(m, B1_Exit());
    EXPECT_CALL(m, B_Exit());
    EXPECT_CALL(m, C1_action(data));
    EXPECT_CALL(m, C_Entry());
    EXPECT_CALL(m, C1_Entry());
    EXPECT_CALL(m, internalAction()).WillOnce(Invoke([this] { EXPECT_EQ(MockMachine::C1, m.getState()); }));

    // When
    m.to_B1();

    // Then
    verifyExpectations();
    EXPECT_EQ(MockMachine::C1, m.getState());
}

namespace {

// http://www.agilemodeling.com/artifacts/stateMachineDiagram.htm, figure 1
class Enrollment
{
public:
    enum State
    {
        PROPOSED,
        SCHEDULED,
        OPEN,
        FULL,
        CLOSED,
        DONE
    };

    Enrollment()
      : machine_(PROPOSED),
        seats_(),
        seatCount_(0),
        time_()
    {
        auto addToWaitingList = [this](const std::string& student) { waitingList_.push_back(student); };
        auto drop = [this](const std::string& student)
            {
                seats_.erase(student);
                if (availableSeats() && !waitingList_.empty())
                {
                    seats_.insert(waitingList_.front());
                    waitingList_.pop_front();
                }
                for (auto it = waitingList_.begin(); it != waitingList_.end(); ++it)
                {
                    if (*it == student)
                    {
                        it = waitingList_.erase(it);
                        break;
                    }
                }
            };
        auto dropFreesSeat = [this](const std::string& student) { return seats_.find(student) != seats_.end() && waitingList_.empty(); };
        auto dropFreeSeatForWaiter = [this](const std::string& student) { return seats_.find(student) != seats_.end() && !waitingList_.empty(); };
        auto seatsAvailable = [this](const std::string&) { return availableSeats() > 0; };
        auto enroll = [this](const std::string& student) { seats_.insert(student); };
        auto announceOpen = [this]{ std::cout << "Open for enrollment with " << availableSeats() << " available seats" << std::endl; };
        auto storeTime = [this](const std::tm& time) { time_ = std::mktime(const_cast<std::tm*>(&time)); };
        auto closeEnrollment = [this]
            {
                std::cout << "Enrollment closed. Scheduled to start " << std::put_time(std::localtime(&time_), "%c") << " with "
                          << seats_.size() << " students: ";
                for (auto& s : seats_) std::cout << s << " ";
                std::cout << std::endl;
                std::cout << "Waiting list: ";
                while (!waitingList_.empty())
                {
                    std::cout << waitingList_.front() << " ";
                    waitingList_.pop_front();
                }
                std::cout << std::endl;
            };


        machine_.onEntry(PROPOSED).invoke([]{ std::cout << "Enrollment proposed." << std::endl; });
        machine_.onTransition(PROPOSED, SCHEDULED, &Enrollment::schedule).invoke(storeTime);
        machine_.onTransition(PROPOSED, DONE, &Enrollment::cancel);

        machine_.onTransition(SCHEDULED, OPEN, &Enrollment::open)
            .when([](int seats) { return seats > 0; })
            .invoke([this](int seats) { seatCount_ = seats; });
        machine_.onTransition(SCHEDULED, DONE, &Enrollment::cancel);

        machine_.onEntry(OPEN).invoke(announceOpen);
        machine_.onTransition(OPEN, OPEN, &Enrollment::enroll).when(seatsAvailable).invoke(enroll);
        machine_.onTransition(OPEN, FULL, &Enrollment::enroll).invoke(addToWaitingList);
        machine_.onTransition(OPEN, CLOSED, &Enrollment::close);
        machine_.onTransition(OPEN, DONE, &Enrollment::cancel);

        machine_.onTransition(FULL, &Enrollment::enroll).invoke(addToWaitingList);
        machine_.onTransition(FULL, OPEN, &Enrollment::drop).when(dropFreesSeat).invoke(drop);
        machine_.onTransition(FULL, &Enrollment::drop).when(dropFreeSeatForWaiter).invoke(drop);
        machine_.onTransition(FULL, &Enrollment::drop).invoke(drop);
        machine_.onTransition(FULL, CLOSED, &Enrollment::close);
        machine_.onTransition(FULL, DONE, &Enrollment::cancel);

        machine_.onEntry(CLOSED).invoke(closeEnrollment);
        machine_.onTransition(CLOSED, DONE, &Enrollment::cancel);
    }

    void schedule(const std::tm& time) { machine_.handle(&Enrollment::schedule, time); }
    void open(int seats) { machine_.handle(&Enrollment::open, seats); }

    void enroll(const std::string& student) { machine_.handle(&Enrollment::enroll, student); }
    void drop(const std::string& student) {  machine_.handle(&Enrollment::drop, student); }

    void close() { machine_.handle(&Enrollment::close); }
    void cancel() { machine_.handle(&Enrollment::cancel); }

    State getState() const
    {
        return machine_.getState();
    }

private:
    int availableSeats() const
    {
        return seatCount_ - seats_.size();
    }

    StateMachine<Enrollment, State> machine_;

    std::set<std::string> seats_;
    int seatCount_;
    std::deque<std::string> waitingList_;

    std::time_t time_;
};
}

TEST(StateMachineTest, enrollmentExample)
{
    Enrollment e;
    EXPECT_EQ(Enrollment::PROPOSED, e.getState());

    std::time_t now = std::time(nullptr);
    e.schedule(*localtime(&now));
    EXPECT_EQ(Enrollment::SCHEDULED, e.getState());

    e.open(3);
    EXPECT_EQ(Enrollment::OPEN, e.getState());

    e.enroll("Mike");
    e.enroll("Tim");
    e.enroll("Jill");
    EXPECT_EQ(Enrollment::OPEN, e.getState());

    e.enroll("Jack");
    EXPECT_EQ(Enrollment::FULL, e.getState());
    e.enroll("John");
    EXPECT_EQ(Enrollment::FULL, e.getState());

    e.drop("Tim");
    EXPECT_EQ(Enrollment::FULL, e.getState());
    e.enroll("Rose");
    EXPECT_EQ(Enrollment::FULL, e.getState());
    e.drop("John");
    EXPECT_EQ(Enrollment::FULL, e.getState());
    e.drop("Mike");
    EXPECT_EQ(Enrollment::FULL, e.getState());
    e.drop("Jill");
    EXPECT_EQ(Enrollment::OPEN, e.getState());

    e.enroll("Don");
    EXPECT_EQ(Enrollment::OPEN, e.getState());
    e.enroll("Dennis");
    EXPECT_EQ(Enrollment::FULL, e.getState());

    e.close();
    EXPECT_EQ(Enrollment::CLOSED, e.getState());

    e.cancel();
    EXPECT_EQ(Enrollment::DONE, e.getState());
}

} // namespace Logic
