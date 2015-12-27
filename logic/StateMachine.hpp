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
#include <iostream>

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

    /**
     * Wrapper to allow state machine to know when event are being handled.
     * Only to be used internally by the state machine.
     */
    class EventWrapper
    {
    public:
        ~EventWrapper();

        /** Chained -> operator to return current state */
        State* operator->() const;

    private:
        friend class StateMachine;
        EventWrapper(State* state, StateMachine& machine);
        EventWrapper(EventWrapper&& other) = default;

        State* state_;
        StateMachine& machine_;
    };

    /** Overloaded -> operator to allow calling state input event semi-directly through the machine */
    EventWrapper operator->();

    /** @return The current state */
    State* const& state() const;

private:
    /** Prevent copy, move and assignment */
    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    /** Helpers to warn about recursive events */
    void eventHandlingStarted();
    void eventHandlingEnded();

    /** Output actions */
    Actions& actions_;

    /** Current state */
    State* state_;

    /** Map for storing state instances */
    using StateMap = std::map<std::type_index, std::unique_ptr<State>>;
    StateMap states_;

    /** */
    int eventHandlingCount_;
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

    /**
     * Default handler for unhandled events.
     * Should be called by deriving base state in its methods
     * identifying the @p event
     */
    void unhandledEvent(const char* event) const;

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
:   actions_(a), state_(nullptr), eventHandlingCount_(0)
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
StateMachine<State, Actions>::EventWrapper::EventWrapper(State* state, StateMachine& machine) :
    state_(state),
    machine_(machine)
{
    machine_.eventHandlingStarted();
}

template <typename State, typename Actions>
StateMachine<State, Actions>::EventWrapper::~EventWrapper()
{
    machine_.eventHandlingEnded();
}

template <typename State, typename Actions>
State* StateMachine<State, Actions>::EventWrapper::operator->() const
{
    return state_;
}

template <typename State, typename Actions>
typename StateMachine<State, Actions>::EventWrapper StateMachine<State, Actions>::operator->()
{
    return EventWrapper(state_, *this);
}

template <typename State, typename Actions>
State* const& StateMachine<State, Actions>::state() const
{
    return state_;
}

template <typename State, typename Actions>
void StateMachine<State, Actions>::eventHandlingStarted()
{
    if (++eventHandlingCount_ > 1)
    {
        std::cerr << "Warning: Recursive event" << std::endl;
    }
}

template <typename State, typename Actions>
void StateMachine<State, Actions>::eventHandlingEnded()
{
    --eventHandlingCount_;
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
void StateAbs<State, Actions>::changeTo()
{
    exit();
    m.template enter<ConcreteState>();
}

template <class State, class Actions>
void StateAbs<State, Actions>::unhandledEvent(const char* event) const
{
    std::cerr << "Warning: Unhandled event \"" << event << "\" in state " << typeid(*this).name() << std::endl;
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
