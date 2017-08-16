/**
 * Test_StateMachine.cpp
 *
 *  Created on: Apr 28, 2015
 *      Author: lasse
 */

#include "../../common/TypeHelpers.hpp"
#include "../StateMachine2.hpp"
#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <map>

namespace Logic2 {
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


class LevelEvents
{
public:
    virtual void bringDown(MoveOnlyParam p) = 0;
    virtual void liftUp(const Param& p) = 0;
    virtual ~LevelEvents() = default;
};

class Actions
{
    LevelEvents* machine_;

public:
    Actions() : machine_(nullptr) {}

    void setMachine(LevelEvents& machine)
    {
        machine_ = &machine;
    }

    void print(const std::string &str) { std::cout << str << std::endl; }
    void lower(MoveOnlyParam p)
    {
        std::cout << "lower:" << p.i_ << std::endl;

        if (!machine_) return;

        if (p.i_ > 1)
        {
            machine_->bringDown(MoveOnlyParam(p.i_ - 1));
        }
    }
    void lift(const Param& p)
    {
        std::cout << "lift:" << p.i_ << std::endl;

        if (!machine_) return;

        if (p.i_ > 1)
        {
            machine_->liftUp(Param(p.i_ - 1));
        }
    }
    void unhandledEvent() { print("unhandled event"); }
};

class LevelState : public StateAbs<LevelState, Actions>
{
public:
    using StateAbs::StateAbs;
    virtual LevelState& bringDown(MoveOnlyParam p) { unhandledEvent("bringDown"); return *this; }
    virtual LevelState& liftUp(const Param& p) { unhandledEvent("liftUp"); return *this; }
    virtual LevelState& poke(const Param&) { unhandledEvent("poke"); return *this; }
    virtual LevelState& test1(int) { unhandledEvent("test"); return *this; }
    virtual LevelState& test2(int*) { unhandledEvent("test"); return *this; }
    virtual LevelState& test3(const int*) { unhandledEvent("test"); return *this; }
    virtual LevelState& test4(const int&) { unhandledEvent("test"); return *this; }
    virtual LevelState& test5(int* const) { unhandledEvent("test"); return *this; }
};

class Low;

class High : public LevelState {
    using LevelState::LevelState;
    void entry() override { a.print("entering High"); }
    LevelState& liftUp(const Param& p) override { a.lift(p); a.print("High");  return get<Low>(); }
    LevelState& bringDown(MoveOnlyParam p) override { a.lower(std::move(p)); a.print("High"); return get<Low>();  }
    LevelState& poke(const Param& p1) override
    {
//        Param b1 = p1;
//        Param b2 = std::move(p2);
        return *this;
    }
    void exit() override { a.print("leaving High");}
};

class Low : public LevelState {
    using LevelState::LevelState;
    void entry() override { a.print("entering Low");}
    LevelState& liftUp(const Param& p) override { a.lift(p); a.print("Low"); return get<High>(); }
    LevelState& bringDown(MoveOnlyParam p) override { a.lower(std::move(p)); a.print("Low"); return get<High>();  }
    LevelState& poke(const Param& p1) override
    {
//        Param b1 = p1;
//        Param b2 = std::move(p2);
        return *this;
    }
    void exit() override { a.print("leaving Low"); }
};

class LevelStateMachine : public LevelEvents, public LevelState::Machine
{
public:
    using LevelState::Machine::Machine;

    void bringDown(MoveOnlyParam p) override { this->operator()(&LevelState::bringDown, std::move(p)); }
    void liftUp(const Param& p) override { this->operator()(&LevelState::liftUp, p); }
};

} // anonymous namespace

TEST(StateMachine2Test, simple)
{
    Actions a;

    LevelStateMachine m(a);
    m.enter<High>();
    a.setMachine(m);
    m.bringDown(MoveOnlyParam(2));
//    m.bringDown(Param(1));
//
    Param p1(100);
    m(&LevelState::poke, p1);

    int i = 0;

    m(&LevelState::test1, 1);
//    m(&LevelState::test2, &i);
    m(&LevelState::test3, &i);
    m(&LevelState::test4, 1);
//    m(&LevelState::test5, &i);

//    Param p2(4);
//    m.liftUp(p2);
//    m.liftUp(Param(4));
    m.liftUp(Param(2));

//    LevelState::Machine m(a);
//    m.enter<High>();
//    m(&LevelState::bringDown);
//    m(&LevelState::bringDown);
//
//    m(&LevelState::liftUp, 1);
//    m(&LevelState::liftUp, 2);
}



TEST(StateMachine2Test, applyTest)
{
    Param p1(1);
    const auto args = new std::tuple<Param>(p1);
//    auto args = std::make_unique<std::tuple<Param>>(p1);

    class A
    {
    public:
        void func(const Param& p)
        {
            std::cout << "A::func " << p.i_ << std::endl;
        }
    };

    std::function<void(A&)> l = [args = std::move(args)](A& obj)
        {
            auto func = [&obj](const auto& ...a) { std::mem_fn(&A::func)(obj, a...); };
            std::experimental::apply(func, *args);
            delete args;
        };

    A aa;
    l(aa);
}

} // namespace Logic2
