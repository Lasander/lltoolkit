/**
 * Test_StateMachine.cpp
 *
 *  Created on: Apr 28, 2015
 *      Author: lasse
 */

#include "../../common/TypeHelpers.hpp"
#include "../StateMachine.hpp"
#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <map>
#include <typeindex>

namespace Logic {
namespace {

class Actions
{
public:
    virtual void print(const std::string &str) { std::cout << str << std::endl; }
    virtual void unhandledEvent() { print("unhandled event"); }
};

struct LevelState : public StateAbs<LevelState, Actions> {
    using StateAbs::StateAbs;
    virtual void bringDown() { a.unhandledEvent(); }
    virtual void liftUp(int) { a.unhandledEvent(); }
};

struct Low;

struct High : public LevelState {
    using LevelState::LevelState;
    void entry() { a.print("entering High"); }
    void liftUp(int) { a.print("already High"); }
    void bringDown() { changeTo<Low>(); }
    void exit() { a.print("leaving High"); }
};

struct Low : public LevelState {
    using LevelState::LevelState;
    void entry() { a.print("entering Low"); }
    void liftUp(int) { changeTo<High>(); }
    void bringDown() { a.print("already Low"); }
    void exit() { a.print("leaving Low"); }
};


class Machine : private LevelState::Machine
{
public:
    Machine(Actions& actions) : LevelState::Machine(actions)
    {
        enter<High>();
    }

    void liftUp(int i) { state()->liftUp(i); }
    void bringDown() { state()->bringDown(); }
};

} // anonymous namespace

TEST(StateMachineTest, machine2)
{
    Actions a;
    Machine m(a);

    m.bringDown();
    m.bringDown();
    m.liftUp(1);
    m.liftUp(2);
}


} // namespace Logic


