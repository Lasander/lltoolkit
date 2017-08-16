#pragma once

#include <memory>
#include <map>
#include <queue>
#include <typeindex>
#include <cassert>
#include <iostream>
#include <tuple>
#include <experimental/tuple> // for apply

namespace Logic2 {

/**
 * State machine implementation for given base state with given output actions.
 *
 * The machine instance will hold one instance of each state. I.e. multiple machines
 * of the same type will have distinct state objects. This potentially allows
 * the states to have a state of their own. This is however not recommended.
 *
 * The state objects are generated dynamically on first entry.
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
     * Template of state machine event functions.
     *
     * - Function must be a member-function of the base state
     * - Function must return the next state
     * - Function can take arbitraty arguments, but they must be copyable/movable
     *
     * This definition is not meant to be used directly. @see operator()
     */
    template <typename ...Args>
    using EventFunc = State&(State::*)(Args...);

    /**
     * Handle input event
     *
     * @param event event function (i.e. &State::event)
     * @param args event arguments, if any
     */
    template<typename ...Args, typename ...Args2>
    void operator()(EventFunc<Args...> event, Args2&&... args);

private:
    /** Grant StateAbs to retrieve state objects */
    template <typename, typename> friend class StateAbs;

    /**
     * @tparam ConcreteState state to retrieve
     * @return a machine state
     * @see StateAbs::get
     */
    template <typename ConcreteState>
    State& get();

private:
    /** Prevent copy, move and assignment */
    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    /**
     * Helper to handle @p eventFunc and check for a need to change state
     */
    template<typename F>
    void handle(F&& eventFunc);

    /**
     * Helper to perform exit/entry actions when switching to @p state
     *
     * Note: Switching to current state will exit and re-enter the state.
     */
    void switchTo(State& state);

    /** Output actions */
    Actions& actions_;

    /** Current state */
    State* state_;

    /** Map for storing state instances */
    using StateMap = std::map<std::type_index, std::unique_ptr<State>>;
    StateMap states_;

    /** Queued events */
    int eventCount_;
    std::queue<std::function<State&(State&)>> events_;
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
     * Retrieve a machine state
     *
     * Used to fetch the next state.
     *
     * @tparam ConcreteState State to retrieve
     * @return state object
     */
    template <typename ConcreteState>
    State& get();

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
:   actions_(a), state_(nullptr), eventCount_(0), events_()
{
}

template <typename State, typename Actions>
template <typename ConcreteState>
void StateMachine<State, Actions>::enter()
{
    switchTo(get<ConcreteState>());
}

template<typename... Args>
struct validateEventParametersTypes : std::true_type { };
template<typename Arg, typename... Args>
struct validateEventParametersTypes<Arg, Args...> :
    std::conditional_t<
        std::conditional_t<
            std::is_reference<Arg>::value,
            std::is_const<std::remove_reference_t<Arg>>, // reference must be reference-to-const
            std::conditional_t<
                std::is_pointer<Arg>::value,
                std::is_const<std::remove_pointer_t<Arg>>, // pointer must be pointer-to-const
                std::true_type> // by-value ok always
            >::value,
            validateEventParametersTypes<Args...>,
            std::false_type> { };

template <typename State, typename Actions>
template<typename ...Args, typename ...Args2>
void StateMachine<State, Actions>::operator()(EventFunc<Args...> event, Args2&&... args)
{
    // Check that event parameters are not meant to be modified as they might be copied before passing to the state
    static_assert(validateEventParametersTypes<Args...>(), "Unsuitable event parameters: by-reference event parameters must be const.");

    if (++eventCount_ > 1)
    {
        // Handle recursive events after current handling is done
        std::cerr << "Warning: Recursive event" << std::endl;

        // Store (copy) the arguments, as they might be out of scope by the time we handle the event
    #if 0
        events_.emplace([event, args...](State& state) mutable -> State& { return (state.*event)(std::forward<Args2>(args)...); });
    #else
        // Note: cannot use unique_ptr as a copyable type is needed for storing in std::function
        auto argsCopy = std::make_shared<std::tuple<std::remove_reference_t<Args2>...>>(std::forward<Args2>(args)...);
        events_.emplace([event, args = argsCopy] (State& state) -> State&
            {
                auto func = [&state, &event](auto&&... a) -> State& { return std::mem_fn(event)(state, std::forward<Args2>(a)...); };
                return std::experimental::apply(func, *args);
            });
    #endif
        return;
    }

    // Handle primary event
    handle([&event, &args...](State& state) -> State& { return (state.*event)(std::forward<Args2>(args)...); });

    // Handle queued events
    while (!events_.empty())
    {
        handle(events_.front());
        events_.pop();
    }

    assert(eventCount_ == 0);
}

template <typename State, typename Actions>
template <typename ConcreteState>
State& StateMachine<State, Actions>::get()
{
    // Find existing state instance or create new
    auto iter = states_.find(std::type_index(typeid(ConcreteState)));
    if (iter == states_.end())
    {
        iter = states_.emplace(std::type_index(typeid(ConcreteState)), Common::make_unique<ConcreteState>(*this, actions_)).first;
    }

    return *iter->second;
}

template <typename State, typename Actions>
template<typename F>
void StateMachine<State, Actions>::handle(F&& eventFunc)
{
    auto& newState = std::forward<F>(eventFunc)(*state_);
    if (&newState != state_)
    {
        switchTo(newState);
    }
    --eventCount_;
}

template <typename State, typename Actions>
void StateMachine<State, Actions>::switchTo(State& state)
{
    if (state_)
    {
        state_->exit();
    }

    // Update current state
    state_ = &state;

    // Execute entry action
    state_->entry();
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
State& StateAbs<State, Actions>::get()
{
    return m.template get<ConcreteState>();
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

} // namespace Logic2
