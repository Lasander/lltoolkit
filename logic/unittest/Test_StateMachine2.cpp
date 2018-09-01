#include "test_util/LogHelpers.hpp"
#include "logic/StateMachine2.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <iostream>
#include <map>
#include <string>

using namespace testing;

namespace Logic {

enum MyState
{
    A, B, C
};

std::ostream& operator<<(std::ostream& os, const MyState& state)
{
    switch (state)
    {
    case A: os << "A"; break;
    case B: os << "B"; break;
    case C: os << "C"; break;
    }

    return os;
}

class MyMachine
{
    StateMachine<MyState> machine_;
    std::string str_{"first"};
public:
    template <typename ...Args>
    using Event = StateMachine<MyState>::Event<Args...>;

    Event<const int&> event1;
    Event<> event2;
    Event<const std::string&> event3;

    MyMachine()
      : machine_(A),
        event1(machine_, "event1"),
        event2(machine_, "event2"),
        event3(machine_, "event3")
    {
        machine_.add(A, event1, A).
            when([](int i) { return i > 0; }).
            invoke([this](int i) mutable
            {
                event1(i-1);
                event3(str_);
                str_ = "rest";
                std::cout << "Event1: positive " << i << std::endl;
            });
        machine_.add(A, event1, A).when([](int i) { return i < 0; }).invoke([](int i) { std::cout << "Event1: negative " << i << std::endl; });
        machine_.add(A, event1, A).invoke([](auto i) { std::cout << "Event1: zero " << i << std::endl; });
        machine_.add(A, event2, B).invoke([] { std::cout << "Event2 " << std::endl; });
        machine_.add(B, event2).invoke([] { std::cout << "Event2 - internal " << std::endl; });
        machine_.add(B, event1, C);
        machine_.add(A, event3, A).invoke([](const std::string& str) { std::cout << "Event3: " << str << std::endl;} );
    }
};

TEST(StateMachine2, basic)
{
    MyMachine machine;
    machine.event1(2);
    machine.event1(-1);
    machine.event1(0);
    machine.event2();
    machine.event2();
    machine.event1(5);
    machine.event2();
}

} // namespace Logic
