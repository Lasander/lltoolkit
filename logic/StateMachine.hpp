/**
 * StateMachine.h
 *
 *  Created on: Apr 28, 2015
 *      Author: lasse
 */

#ifndef LOGIC_STATEMACHINE_HPP_
#define LOGIC_STATEMACHINE_HPP_

#include <memory>
#include <map>
#include <typeindex>
#include <cassert>

namespace Logic {

/**
 * State machine implementation for given base state with given output actions.
 * The machine instance will hold one instance of each state generated on first
 * entry.
 *
 * @tparam State State type
 * @tparam Actions Output action type
 *
 * Type should be accessed through the State by State::Machine.
 * @see StateAbs
 */
template <typename State, typename Actions>
class StateMachine
{
public:
    /**
     * Construct a state machine
     *
     * @param actions Output actions
     */
    StateMachine(Actions& actions);

    /** Dtor */
    ~StateMachine() = default;

    /**
     * Enter an initial state, or force jump to the given state.
     *
     * @tparam ConcreteState State to enter
     */
    template <typename ConcreteState>
    void enter();

    /** @return The current state */
    State* const& state() const;

private:
    /** Prevent copy, move and assignment */
    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    /** Output actions */
    Actions& actions_;

    /** Current state */
    State* state_;

    /** Map for storing state instances */
    using StateMap = std::map<std::type_index, std::unique_ptr<State>>;
    StateMap states_;
};

/**
 * Generic state type. Derive generic state to define a base state with input interface (events).
 * Further concrete states should be derived from the base state.
 *
 * Derived classes (base state and concrete states) should inherit the constructor by:
 * 'using StateAbs::StateAbs' and 'using BaseState::BaseState', respectively.
 *
 * @tparam State (Derived) state type
 * @tparam Actions Output action type
 */
template <class State, class Actions>
class StateAbs
{
public:
    /** Machine for this generic state */
    using Machine = StateMachine<State, Actions>;

    /**
     * Constructor. Not meant to be called directly.
     *
     * @param machine State machine implementation,
     *                needed for state changing.
     * @param actions Output actions
     */
    explicit StateAbs(Machine& machine, Actions& actions);

    /** Virtual dtor */
    virtual ~StateAbs();

protected:
    /**
     * Change to given state.
     *
     * @tparam ConcreteState State to change to
     *
     * Entering the current state will re-enter the state, i.e.
     * execute exit/entry.
     */
    template <typename ConcreteState>
    void changeTo();

    /** Output actions to be utilized by concrete states */
    Actions& a;

private:
    /** Grant Machine access to call entry/exit */
    friend Machine;

    /** State entry action. Default actions does nothing. */
    virtual void entry();

    /** State exit action. Default actions does nothing. */
    virtual void exit();

    /** The associated machine */
    Machine& m;
};

/** StateMachine implementation */

template <typename State, typename Actions>
StateMachine<State, Actions>::StateMachine(Actions& a)
:   actions_(a), state_(nullptr)
{
}

template <typename State, typename Actions>
template <typename ConcreteState>
void StateMachine<State, Actions>::enter()
{
    // Find existing state instance or create new
    auto iter = states_.find(std::type_index(typeid(ConcreteState)));
    if (iter == states_.end())
    {
        iter = states_.emplace(std::type_index(typeid(ConcreteState)), Common::make_unique<ConcreteState>(*this, actions_)).first;
    }

    // Update current state
    state_ = iter->second.get();

    // Execute entry actions
    state_->entry();
}

template <typename State, typename Actions>
State* const& StateMachine<State, Actions>::state() const
{
    return state_;
}


/** StateAbs implementation */

template <class State, class Actions>
StateAbs<State, Actions>::StateAbs(typename StateAbs<State, Actions>::Machine& m, Actions& a)
:   a(a), m(m)
{
}

template <class State, class Actions>
StateAbs<State, Actions>::~StateAbs()
{
}

template <class State, class Actions>
template <typename ConcreteState>
void StateAbs<State, Actions>::changeTo() {
    exit();
    m.template enter<ConcreteState>();
}

template <class State, class Actions>
void StateAbs<State, Actions>::entry()
{
}

template <class State, class Actions>
void StateAbs<State, Actions>::exit()
{
}



} // namespace Logic

#endif /* LOGIC_STATEMACHINE_HPP_ */
