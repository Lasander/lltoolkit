/**
 * Test_StateMachine.cpp
 *
 *  Created on: Apr 28, 2015
 *      Author: lasse
 */

#include "../../common/TypeHelpers.hpp"
#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <map>
#include <iomanip>
#include <future>
#include "../StateMachine.hpp"

namespace Logic {
namespace {

class Param
{
public:
    explicit Param(int i) : i_(i)
    {
        std::cout << "ctor: " << i_ << std::endl;
    }

    ~Param()
    {
        std::cout << "dtor: " << i_ << std::endl;
        i_ = -i_;
    }

    Param(const Param& o)
      : i_(o.i_)
    {
        std::cout << "copy: " << i_ << std::endl;
    }

    Param& operator==(const Param& o)
    {
        i_ = o.i_;
        std::cout << "copy assignment: " << i_ << std::endl;
        return *this;
    }

    Param(Param&& o)
      : i_(o.i_)
    {
        o.i_ = -o.i_;
        std::cout << "move: " << i_ << std::endl;
    }
    Param& operator==(Param&& o)
    {
        i_ = o.i_;
        o.i_ = -o.i_;
        std::cout << "move assignment: " << i_ << std::endl;
        return *this;
    }

    int i_;
};

class MoveOnlyParam
{
public:
    explicit MoveOnlyParam(int i) : i_(i)
    {
        std::cout << "ctor: " << i_ << std::endl;
    }

    ~MoveOnlyParam()
    {
        std::cout << "dtor: " << i_ << std::endl;
        i_ = -i_;
    }

    MoveOnlyParam(const MoveOnlyParam&) = delete;
    MoveOnlyParam& operator==(const MoveOnlyParam&) = delete;

    MoveOnlyParam(MoveOnlyParam&& o)
      : i_(o.i_)
    {
        o.i_ = -o.i_;
        std::cout << "move: " << i_ << std::endl;
    }
    MoveOnlyParam& operator==(MoveOnlyParam&& o)
    {
        i_ = o.i_;
        o.i_ = -o.i_;
        std::cout << "move assignment: " << i_ << std::endl;
        return *this;
    }

    int i_;
};


enum MyState
{
    A,
    B,
    C
};

class MyMachine : private StateMachine<MyMachine, MyState>
{
public:
    MyMachine()
      : StateMachine<MyMachine, MyState>(A)
    {
        onTransition(A, B, &MyMachine::first).invoke([](int i) { std::cout << "A->B(" << i << ")" << std::endl; });
        onTransition(B, A, &MyMachine::first).invoke([](int i) { std::cout << "B->A(" << i << ")" << std::endl; });
        onTransition(A, C, &MyMachine::second);
        onTransition(C, A, &MyMachine::first);
        onTransition(A, A, &MyMachine::test1).invoke([](const Param& p) { std::cout << "test1(" << p.i_ << ")" << std::endl; });
        onTransition(A, A, &MyMachine::test2).invoke([](const MoveOnlyParam& p) { std::cout << "test2(" << p.i_ << ")" << std::endl; });
    }

    ~MyMachine()
    {
    }

    void first(int i) { handle(&MyMachine::first, i); };
    void second() { handle(&MyMachine::second); };
    void test1(const Param& p) { handle(&MyMachine::test1, p); };
    void test2(MoveOnlyParam&& p) { handle(&MyMachine::test2, std::move(p)); };
};

class MyMachine2
{
public:
    MyMachine2()
      : impl_(A)
    {
        impl_.onTransition(A, B, &MyMachine2::first).invoke([](int i) { std::cout << "A->B(" << i << ")" << std::endl; });
        impl_.onTransition(B, A, &MyMachine2::first).invoke([](int i) { std::cout << "B->A(" << i << ")" << std::endl; });
        impl_.onTransition(A, C, &MyMachine2::second);
        impl_.onTransition(C, A, &MyMachine2::first);
        impl_.onTransition(A, A, &MyMachine2::test1).invoke([](const Param& p) { std::cout << "test1(" << p.i_ << ")" << std::endl; });
        impl_.onTransition(A, A, &MyMachine2::test2).invoke([](const MoveOnlyParam& p) { std::cout << "test2(" << p.i_ << ")" << std::endl; });

        impl_.onEntry(A).invoke([]{ std::cout << "Enter A" << std::endl; });
        impl_.onExit(A).invoke([]{ std::cout << "Exit A" << std::endl; });
        impl_.onEntry(C).invoke([]{ std::cout << "Enter C" << std::endl; });
        impl_.onExit(C).invoke([]{ std::cout << "Exit C" << std::endl; });

        impl_.onTransition(C, &MyMachine2::second).invoke([]{ std::cout << "Staying in C" << std::endl; });
    }

    enum State
    {
        A,
        B,
        C
    };

    void first(int i) { impl_(&MyMachine2::first, i); };
    void second() { impl_(&MyMachine2::second); };
    void test1(const Param& p) { impl_(&MyMachine2::test1, p); };
    void test2(MoveOnlyParam&& p) { impl_(&MyMachine2::test2, std::move(p)); };

private:
    StateMachine<MyMachine2, State> impl_;
};

} // anonymous namespace

TEST(StateMachineTest, simple)
{
    MyMachine m;
    m.second();
    m.first(1);
    m.first(2);
    m.first(3);

    m.test1(Param(1));
    MoveOnlyParam p2(2);
    m.test2(std::move(p2));
}

TEST(StateMachineTest, simple2)
{
    MyMachine2 m;
    m.second();
    m.second();
    m.first(1);
    m.first(2);
    m.first(3);

    m.test1(Param(1));
    m.test2(MoveOnlyParam(2));
}

TEST(StateMachineTest, simpleExternal)
{
    StateMachine<MyMachine, MyState> m(A);
    m.onTransition(A, B, &MyMachine::first).invoke([](int i) { std::cout << "A->B(" << i << ")" << std::endl; });
    m.onTransition(B, A, &MyMachine::first).invoke([](int i) { std::cout << "B->A(" << i << ")" << std::endl; });
    m.onTransition(A, C, &MyMachine::second);
    m.onTransition(C, A, &MyMachine::first);
    m.onTransition(A, A, &MyMachine::test1).invoke([](const Param& p) { std::cout << "test1(" << p.i_ << ")" << std::endl; });
    m.onTransition(A, A, &MyMachine::test2).invoke([](const MoveOnlyParam& p) { std::cout << "test2(" << p.i_ << ")" << std::endl; });

    m(&MyMachine::second);
    m(&MyMachine::first, 1);
    m(&MyMachine::first, 2);
    m(&MyMachine::first, 3);
    m(&MyMachine::test1, Param(1));
    m(&MyMachine::test2, MoveOnlyParam(2));
}

TEST(StateMachineTest, conditionalTransition)
{
    StateMachine<MyMachine, MyState> m(A);
    m.onTransition(A, A, &MyMachine::first)
            .when([](int i) { return i == 2; })
            .invoke([](int i) { std::cout << "i == 2 first(" << i << ")" << std::endl; });
    m.onTransition(A, A, &MyMachine::first)
            .when([](int i) { return i > 1; })
            .invoke([](int i) { std::cout << "i > 1 first(" << i << ")" << std::endl; });
    m.onTransition(A, A, &MyMachine::first)
            .invoke([](int i) { std::cout << "unconditional first(" << i << ")" << std::endl; });

    m(&MyMachine::first, 1);
    m(&MyMachine::first, 2);
    m(&MyMachine::first, 3);
}

TEST(StateMachineTest, test)
{
    using EventFunc = void(MyMachine::*)(int);

    {
        MyMachine m;
        EventFunc f = &MyMachine::first;
        (m.*f)(1);

        std::function<void(MyMachine&, int)> f2(std::mem_fn(&MyMachine::first));
        f2(m, 2);

        std::shared_ptr<void> f6 = std::make_shared<decltype(std::mem_fn(&MyMachine::first))>(std::mem_fn(&MyMachine::first));
        auto f7 = std::static_pointer_cast<decltype(std::mem_fn(&MyMachine::first))>(f6);
        (*f7)(m, 7);
    }

    std::shared_ptr<void> func(new Param(21));
}

TEST(StateMachineTest, recursion)
{
    class MyMachine
    {
    public:
        MyMachine()
          : impl_(A)
        {
            impl_.onTransition(A, B, &MyMachine::first).invoke([this](const MoveOnlyParam& p)
                {
                    std::cout << "A->B(" << p.i_ << ")" << std::endl;
                    if (p.i_ > 0)
                    {
                        first(MoveOnlyParam(-(p.i_ - 1)));
                        first(MoveOnlyParam(-(p.i_ - 2)));
                    }
                });
            impl_.onTransition(B, A, &MyMachine::first).invoke([this](const MoveOnlyParam& p)
                {
                    std::cout << "B->A(" << p.i_ << ")" << std::endl;
                    if (p.i_ < 0)
                    {
                        first(MoveOnlyParam(-(p.i_ + 1)));
                        first(MoveOnlyParam(-(p.i_ + 2)));
                    }
                });
        }

        ~MyMachine()
        {
        }

        void first(MoveOnlyParam&& p) { std::cout << "first(" << p.i_ << ")" << std::endl; impl_(&MyMachine::first, std::move(p)); };

    private:
        StateMachine<MyMachine, MyState> impl_;
    };

    MyMachine m;
    m.first(MoveOnlyParam(3));
}

namespace
{

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

TEST(StateMachineTest, enrollmentMachine)
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

template<typename T, typename R, typename ...Args>
auto memberCall(T& obj, R(T::*func)(Args...))
{
    return [&obj, func](Args... args) { (obj.*func)(std::forward<Args...>(args)...); };
}
template<typename T, typename R, typename ...Args>
auto memberCall(const T& obj, R(T::*func)(Args...) const)
{
    return [&obj, func](Args... args) { (obj.*func)(std::forward<Args...>(args)...); };
}

TEST(StateMachineTest, enrollmentMachineNewSyntax)
{
    class MyMachine
    {
    public:
        MyMachine()
          : impl_(A)
        {
            impl_.onTransition(A, B, &MyMachine::first).invoke(*this, &MyMachine::doFirst);
            impl_.onTransition(A, B, &MyMachine::first)
                    .when([](int i){ return i != 0; })
                    .invoke([](int i){ std::cout << i << std::endl;  return i; });
            impl_.onTransition(B, A, &MyMachine::first).when([](int i){ return i != 0; }).invoke([](int i){ std::cout << i << std::endl;  return i; });
        }

        ~MyMachine()
        {
        }

        void first(int i) { impl_(&MyMachine::first, i); };

        int doFirst(int i) const
        {
            std::cout << i << std::endl;
            return 1;
        }

    private:
        StateMachine<MyMachine, MyState> impl_;
    };

    MyMachine m;
    m.first(1);
    m.first(0);
    m.first(2);


//    Machine<MyMachine, MyState>::Transformer<int> t(A, A, &MyMachine::first);
//    t << [](int i){ std::cout << i << std::endl; };
}

namespace M {
class T
{
protected:
    int current_;
    int event_;
    int condition_;
    int action_;
    int next_;

    T(T&&) = default;

    template <typename Concrete>
    Concrete& merge(const T& o)
    {
        current_ = std::max(current_, o.current_);
        event_ = std::max(event_, o.event_);
        condition_ = std::max(condition_, o.condition_);
        action_ = std::max(action_, o.action_);
        next_ = std::max(next_, o.next_);
        return static_cast<Concrete&>(*this);
    }

public:
    T() : current_(-1), event_(-1), condition_(-1), action_(-1), next_(-1) { }

    void print() const
    {
        std::cout << current_ << event_ << condition_ << action_ << next_ << std::endl;
    }
};

class S : public T
{
public:
    S(int i) : T()
    {
        current_ = i;
    }

    auto operator+(const T& t) -> decltype(*this)
    {
        std::cout << "+" << std::endl;
        return merge<decltype(*this)>(t);
    }
    auto operator==(const T& t) -> decltype(*this)
    {
        std::cout << "==" << std::endl;
        return merge<decltype(*this)>(t);
    }
};
class E : public T
{
public:
    E(int i) : T()
    {
        event_ = i;
    }
    auto operator[](const T& t) -> decltype(*this)
    {
        std::cout << "[]" << std::endl;
        return merge<decltype(*this)>(t);
    }
    auto operator/(const T& t) -> decltype(*this)
    {
        std::cout << "/" << std::endl;
        return merge<decltype(*this)>(t);
    }
};
class C : public T
{
public:
    C(int i) : T()
    {
        condition_ = i;
    }
};
class A : public T
{
public:
    A(int i) : T()
    {
        action_ = i;
    }
};
class N : public T
{
public:
    N(int i) : T()
    {
        next_ = i;
    }
};

} // namespace M

TEST(StateMachineTest, operatorSyntaxEnforcement)
{
//    A + B * C / D [E] == F;

    const M::T& c = M::S(1) + M::E(2) [M::C(3)] / M::A(4) == M::N(5);
    c.print();

    const M::T& c2 = M::S(1) + M::E(2) / M::A(4) == M::N(5);
    c2.print();

    const M::T& c3 = M::S(1) + M::E(2) / M::A(4);
    c3.print();
}

TEST(StateMachineTest, threads)
{
    class C
    {
    public:
        void g(const std::string& str)
        {
            std::cout << str << std::endl;
        }

        void f()
        {
            std::string temp("a string");

            auto future = std::async(&C::g, this, temp);
            future.get();
        }

    };

    C c;
    c.f();
}

namespace A{

class Machine
{
public:
    Machine()
      : impl_(A11)
    {
        impl_.setParent(A, {A1, A2});
        impl_.setParent(A1, A11);
        impl_.setParent(B, B1);

        impl_.onEntry(A).invoke([]{ std::cout << "Enter A" << std::endl; });
        impl_.onExit(A).invoke([]{ std::cout << "Exit A" << std::endl; });
        impl_.onEntry(A1).invoke([]{ std::cout << "Enter A1" << std::endl; });
        impl_.onExit(A1).invoke([]{ std::cout << "Exit A1" << std::endl; });
        impl_.onEntry(A11).invoke([]{ std::cout << "Enter A11" << std::endl; });
        impl_.onExit(A11).invoke([]{ std::cout << "Exit A11" << std::endl; });
        impl_.onEntry(A2).invoke([]{ std::cout << "Enter A2" << std::endl; });
        impl_.onExit(A2).invoke([]{ std::cout << "Exit A2" << std::endl; });
        impl_.onEntry(B).invoke([]{ std::cout << "Enter B" << std::endl; });
        impl_.onExit(B).invoke([]{ std::cout << "Exit B" << std::endl; });
        impl_.onEntry(B1).invoke([]{ std::cout << "Enter B1" << std::endl; });
        impl_.onExit(B1).invoke([]{ std::cout << "Exit B1" << std::endl; });

        impl_.onTransition(A11, B1, &Machine::to_B1);
        impl_.onTransition(B1, A11, &Machine::to_A11);
    }

    enum State
    {
        A,
          A1,
            A11,
          A2,
        B,
          B1
    };

    void to_A11() { impl_(&Machine::to_A11); };
    void to_B1() { impl_(&Machine::to_B1); };

private:
    StateMachine<Machine, State> impl_;
};
}

TEST(StateMachineTest, hierarchy)
{
    A::Machine m;
//    m.to_B1();
//    m.to_A11();
}

} // namespace Logic
