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

namespace Logic {
namespace {

class Actions
{
public:
    void print(const std::string &str) { std::cout << str << std::endl; }
    void unhandledEvent() { print("unhandled event"); }
};

struct LevelState : public StateAbs<LevelState, Actions> {
    using StateAbs::StateAbs;
    virtual void bringDown() { unhandledEvent("bringDown"); }
    virtual void liftUp(int) { unhandledEvent("liftUp"); }
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
    void exit() { a.print("leaving Low"); }
};

} // anonymous namespace

TEST(StateMachineTest, simple)
{
    Actions a;
    LevelState::Machine m(a);
    m.enter<High>();

    m->bringDown();
    m->bringDown(); // unhandled event
    m->liftUp(1);
    m->liftUp(2);
}


} // namespace Logic
