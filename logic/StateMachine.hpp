#pragma once

#include <cassert>
#include <iostream>
#include <map> // for (ordered) multimap
#include <queue>
#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace Logic {

/**
 * State machine implementation defined by:
 * @tparam ConcreteMachine Machine with function for input events
 * @tparam State State type (enumeration)
 *
 * Supported features:
 * - Transitions between states on events
 * - State-internal transitions
 * - Guard conditions on transitions
 * - Transition actions
 * - State entry/exit actions
 * - State hierarchy
 * - Recursive events (event calls resulting from actions)
 *     - Recursive calls are queued and handled after completing
 *       the handling of the current event
 *     - Note: As any event could be recursive, all event parameters
 *       must be copyable or movable.
 *
 * TODO:
 * - Make true composite states instead of hierarchically defined state behavior
 * - Add support for composite state history pseudo-states
 * - Consider orthogonal regions
 */
template <typename ConcreteMachine, typename State>
class StateMachine
{
    /** Type for stored action/condition functions */
    using StoredFunction = std::shared_ptr<void>;

public:
    /** Construct a machine in initial @p state */
    StateMachine(State state);
    ~StateMachine() = default;

    /**
     * Event function type:
     * - a member function of the ConcreteMachine,
     * - taking arbitrary parameters, and
     * - returning void
     *
     * @see handle
     */
    template <typename ...Args>
    using EventFunc = void(ConcreteMachine::*)(Args...);

    /**
     * Explicitly enter the initial state executing any necessary (hierarchical) entry actions
     *
     * Should be called after the machine hierarchy and transactions have been fully defined.
     * Will have an effect only on first call on each instance.
     */
    void enterInitialState();

    /**
     * Handle @p event with @p args.
     *
     * Can be called directy of from the event functions in the ConcreteMachine. E.g.
     *
     * class MyMachine
     * {
     * public:
     *     ...
     *     void event(int param) { impl.handle(&MyMachine::event, param); }
     * private:
     *     Machine<MyMachine, State> impl;
     * };
     *
     * Any recursive events caused by actions of the transitions will be queued and
     * handled after the previous event has been fully handled.
     */
    template<typename ...Args, typename ...Args2>
    void handle(EventFunc<Args...> event, Args2&&... args);

    /** Shortcut for @see handle */
    template<typename ...Args, typename ...Args2>
    void operator()(EventFunc<Args...> event, Args2&&... args);

    /**
     * Builder for machine transitions
     *
     * Create with @see onTransition.
     * Add event condition and/or action using the member functions, e.g.
     *
     *   onTransition(START, NEXT, &MyMachine::event).when(<condition>).invoke(<action>)
     */
    template<typename ...Args>
    class TransitionBuilder
    {
    public:
        /**
         * @name Add transition condition
         *
         * Condition must:
         * - be a callable with the event parameters
         * - return value convertible to boolean
         */
        ///@{
        template<typename Condition>
        auto when(Condition&& action) -> TransitionBuilder&;
        template<typename T, typename R>
        auto when(T& obj, R(T::*function)(Args...)) -> TransitionBuilder&;
        template<typename T, typename R>
        auto when(const T& obj, R(T::*function)(Args...) const) -> TransitionBuilder&;
        ///@}

        /**
         * @name Add transition action
         *
         * Action must:
         * - be a callable with the event parameters
         *
         * Note: any return value from action is ignored.
         */
        ///@{
        template<typename Action>
        void invoke(Action&& action);
        template<typename T, typename R>
        void invoke(T& obj, R(T::*function)(Args...));
        template<typename T, typename R>
        void invoke(const T& obj, R(T::*function)(Args...) const);
        ///@}

        ~TransitionBuilder();
        TransitionBuilder(const TransitionBuilder&) = delete;
        TransitionBuilder& operator=(const TransitionBuilder&) = delete;

    private:
        // Allow Machine to create and move
        friend class StateMachine;

        TransitionBuilder(StateMachine& machine, State current, State next, EventFunc<Args...> event, bool internal);
        TransitionBuilder(TransitionBuilder&&) = default;

    private:
        StateMachine& machine_;
        State current_;
        State next_;
        EventFunc<Args...> event_;
        StoredFunction condition_;
        StoredFunction action_;
        bool internal_;
    };

    /**
     * Add transition from @p current to @p next on @p event
     *
     * @see TransitionBuilder for adding conditions and actions to the transition
     * Transition to self will cause state to be re-entered.
     */
    template<typename ...Args>
    auto onTransition(State current, State next, EventFunc<Args...> event) -> TransitionBuilder<Args...>;
    /**
     * Add internal transition in @p current on @p event
     *
     * @see TransitionBuilder for adding conditions and actions to the transition
     * State will not be re-entered.
     */
    template<typename ...Args>
    auto onTransition(State current, EventFunc<Args...> event) -> TransitionBuilder<Args...>;

    /**
     * Builder for extry and exit actions
     *
     * Create with @see onEntry or @see onExit
     * Add the action using the invoke member functions, e.g.
     *
     *   onEntry(START).invoke(<action>)
     */
    class EntryExitActionBuilder
    {
    public:
        /**
         * @name Add action
         *
         * Action must:
         * - be a callable without parameter
         *
         * Note: any return value from action is ignored.
         */
        ///@{
        template<typename Action>
        void invoke(Action&& action);
        template<typename T, typename R>
        void invoke(T& obj, R(T::*function)());
        template<typename T, typename R>
        void invoke(const T& obj, R(T::*function)() const);
        ///@}

        ~EntryExitActionBuilder();
        EntryExitActionBuilder(const EntryExitActionBuilder&) = delete;
        EntryExitActionBuilder& operator=(const EntryExitActionBuilder&) = delete;

    private:
        // Allow Machine to create and move
        friend class StateMachine;

        EntryExitActionBuilder(StateMachine& machine, State current, bool entry);
        EntryExitActionBuilder(EntryExitActionBuilder&&) = default;

    private:
        StateMachine& machine_;
        State current_;
        bool entry_;
        std::function<void()> action_;
    };

    /**
     * Add entry action for @p state
     *
     * Use invoke() methods on the returned object to add the desired action.
     *
     * In case @p state is the initial state and no events have been handled the
     * action will be performed.
     */
    auto onEntry(State state) -> EntryExitActionBuilder;

    /**
     * Add exit action for @p state
     *
     * Use invoke() methods on the returned object to add the desired action.
     */
    auto onExit(State state) -> EntryExitActionBuilder;

    /** Set @p parent state for @p children */
    void setParent(State parent, const std::set<State>& children);

    /** Set @p parent state for single @p child */
    void setParent(State parent, State child);

    /** @return current state */
    State getState() const;

private:
    /** Action function type */
    template <typename ...Args>
    using ActionFunc = std::function<void(Args...)>;

    /** Condition function type */
    template <typename ...Args>
    using ConditionFunc = std::function<bool(Args...)>;

    template<typename ...Args>
    void addTransition(
        State current, State next, EventFunc<Args...> event,
        StoredFunction condition, StoredFunction action,
        bool internal);

    void addEntryAction(State state, std::function<void()> action);
    void addExitAction(State state, std::function<void()> action);

    /** Helper to execute one event. @see handle */
    template<typename ...Args, typename ...Args2>
    void execute(EventFunc<Args...> event, Args2&&... args);

    /** Execute state entry action, if any */
    void enter(State state);
    /** Execute state exit action, if any */
    void exit(State state);

    /** Current state */
    State state_;
    /** Parent states */
    std::unordered_map<State, State> parent_;

    /**
     * @return all ancestors of @p state including state itself
     *
     * I.e. state -> parent of state -> parent of parent of state -> ...
     */
    std::vector<State> getAncestors(State state) const;

    /**
     * @return ancestors of @p state including state itself up to, but
     *         excluding, first common ancestor with @p referenceState
     *
     * E.g. if state A ancestors are B -> C and referenceState ancestors
     * are C -> D, the return value will be (A, B).
     *
     * In the special case the state == referenceState, return value will
     * contain only state.
     */
    std::vector<State> getAncestorsUntilCommonAncestor(State state, State referenceState) const;

    /**
     * Encapsulate a transition [current state + event -> next state + action]
     *
     * Unique transition is defined by a (current state, event) pair.
     */
    class Transition
    {
    public:
        /**
         * Construct new transition
         *
         * @param current Start state
         * @param next New state
         * @param event Trigger event
         * @param condition Transition condition, nullptr for no condition
         * @param action Transition action, nullptr for no action
         * @param internal True if this is actually an internal action rather
         *                 than a true transition. Internal action does not
         *                 change the state (or cause current state to be
         *                 re-entered), but is most easily modeled as a
         *                 (custom) transition to self.
         */
        template <typename ...Args>
        Transition(
            State current, State next, EventFunc<Args...> event,
            StoredFunction condition, StoredFunction action,
            bool internal);

        ~Transition() = default;
        Transition(const Transition&) = delete;
        Transition& operator=(const Transition&) = delete;
        Transition(Transition&& t) = default;

        /**
         * @return true if this transition changes state, i.e. should exit/entry actions
         *         be performed then executing this transition.
         */
        bool changesState() const;

        /** @return new state after this transition */
        State getNextState() const;

        /** @return transition condition result for @p event with @p args */
        template<typename ...Args, typename ...Args2>
        bool checkCondition(EventFunc<Args...> event, Args2&&... args) const;

        /**
         * Execute transition for @p event with @p args
         * @return new state
         */
        template<typename ...Args, typename ...Args2>
        State execute(EventFunc<Args...> event, Args2&&... args) const;

        /** Type for unique id for the transition (within this machine instance) */
        using Id = std::pair<State, int>;

        /** @return unique id of this transition */
        Id getIdentifier() const;

        /** @return unique transition id based on state and event */
        template <typename ...Args>
        static Id createIdentifier(State state, EventFunc<Args...> event);

        /**
         * @return true if this transition is "less-than" @p other
         *
         * Needed for Transition sortability required for storage in a set.
         */
        bool operator<(const Transition& other) const;

    private:
        /** @return unique identifier for @p event in this machine instance */
        template <typename ...Args>
        static int identify(EventFunc<Args...> event);
        /** Helper used by @see identify */
        static int getEventIndex();

        State current_;
        State next_;
        int event_;
        StoredFunction action_;
        StoredFunction condition_;
        bool internal_;
    };

    /** Registered transitions */
    using TransitionContainer = std::multimap<typename Transition::Id, const Transition>;
    TransitionContainer transitions_;

    /**
     * Find transition for @p event with @p args in the current state
     *
     * Will consider state ancestors to handle the event if necessary.
     *
     * @return iterator to transitions_ pointing to found transition or to end() if transition was not found.
     */
    template<typename ...Args, typename ...Args2>
    auto findTransition(EventFunc<Args...> event, Args2&&... args) -> typename TransitionContainer::iterator;

    /** Entry and exit action function type */
    using EntryExitActionFunc = std::function<void()>;

    /** Entry actions */
    std::unordered_map<State, EntryExitActionFunc> entryActions_;

    /**
     * Keep track whether the initial state entry action (if any) has been executed.
     * The action will be executed (once) when explicitly invoked (@see enterInitialState)
     * or on first event to the machine.
     */
    bool initialEntryExecuted_;

    /** Exit actions */
    std::unordered_map<State, EntryExitActionFunc> exitActions_;

    /** Queued events */
    int eventCount_;
    std::queue<std::function<void()>> events_;
};

template <typename ConcreteMachine, typename State>
StateMachine<ConcreteMachine, State>::StateMachine(State state)
  : state_(state),
    parent_(),
    transitions_(),
    entryActions_(),
    initialEntryExecuted_(false),
    exitActions_(),
    eventCount_(),
    events_()
{
}

template <typename ConcreteMachine, typename State>
void StateMachine<ConcreteMachine, State>::enterInitialState()
{
    if (!initialEntryExecuted_)
    {
        initialEntryExecuted_ = true;

        const auto newAncestors = getAncestors(state_);
        for (auto it = newAncestors.rbegin(); it != newAncestors.rend(); ++it)
        {
            enter(*it);
        }
    }
}

// Implement std::apply from c++17 using c++14
namespace std17 {
namespace details {
template <typename F, typename Tuple, std::size_t...Is>
decltype(auto) apply(std::index_sequence<Is...>, F&& f, Tuple&& tuple)
{
    return std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(tuple))...);
}
} // namespace details

template <typename F, typename Tuple>
decltype(auto) apply(F&& f, Tuple&& tuple)
{
    return std17::details::apply(std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{}, std::forward<F>(f), std::forward<Tuple>(tuple));
}
} // namespace std17

template <typename ConcreteMachine, typename State>
template<typename ...Args, typename ...Args2>
void StateMachine<ConcreteMachine, State>::handle(EventFunc<Args...> event, Args2&&... args)
{
    if (!initialEntryExecuted_)
    {
        enterInitialState();
    }

    if (++eventCount_ > 1)
    {
        // Handle recursive events after current handling is done
        // Store (copy) the arguments, as they might be out of scope by the time we handle the event
        // Note: cannot use unique_ptr as a copyable type is needed for storing in std::function
        auto storedArgs = std::make_shared<std::tuple<std::remove_reference_t<Args2>...>>(std::forward<Args2>(args)...);
        auto storedFunc = [this, event, storedArgs]
            {
                auto execFunc = [this, event](auto&&... a) mutable { execute(event, std::forward<Args2>(a)...); };
                std17::apply(execFunc, *storedArgs);
            };
        events_.emplace(storedFunc);
        return;
    }

    // Handle primary event
    execute(event, std::forward<Args2>(args)...);

    // Handle queued events
    while (!events_.empty())
    {
        events_.front()();
        events_.pop();
    }

    assert(eventCount_ == 0);
}

template <typename ConcreteMachine, typename State>
template<typename ...Args, typename ...Args2>
void StateMachine<ConcreteMachine, State>::operator()(EventFunc<Args...> event, Args2&&... args)
{
    handle(event, std::forward<Args2>(args)...);
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
template<typename Condition>
auto StateMachine<ConcreteMachine, State>::TransitionBuilder<Args...>::when(Condition&& action) -> TransitionBuilder&
{
    condition_ = std::make_shared<ConditionFunc<Args...>>(std::forward<Condition>(action));
    return *this;
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
template<typename T, typename R>
auto StateMachine<ConcreteMachine, State>::TransitionBuilder<Args...>::when(T& obj, R(T::*function)(Args...)) -> TransitionBuilder&
{
    auto f = [&obj, function](Args... args) -> bool { return (obj.*function)(std::forward<Args>(args)...); };
    condition_ = std::make_shared<ActionFunc<Args...>>(f);
    return *this;
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
template<typename T, typename R>
auto StateMachine<ConcreteMachine, State>::TransitionBuilder<Args...>::when(const T& obj, R(T::*function)(Args...) const) -> TransitionBuilder&
{
    auto f = [&obj, function](Args... args) -> bool { return (obj.*function)(std::forward<Args>(args)...); };
    condition_ = std::make_shared<ActionFunc<Args...>>(f);
    return *this;
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
template<typename Action>
void StateMachine<ConcreteMachine, State>::TransitionBuilder<Args...>::invoke(Action&& action)
{
    action_ = std::make_shared<ActionFunc<Args...>>(std::forward<Action>(action));
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
template<typename T, typename R>
void StateMachine<ConcreteMachine, State>::TransitionBuilder<Args...>::invoke(T& obj, R(T::*function)(Args...))
{
    auto f = [&obj, function](Args... args){ (obj.*function)(std::forward<Args>(args)...); };
    action_ = std::make_shared<ActionFunc<Args...>>(f);
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
template<typename T, typename R>
void StateMachine<ConcreteMachine, State>::TransitionBuilder<Args...>::invoke(const T& obj, R(T::*function)(Args...) const)
{
    auto f = [&obj, function](Args... args){ (obj.*function)(std::forward<Args>(args)...); };
    action_ = std::make_shared<ActionFunc<Args...>>(f);
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
StateMachine<ConcreteMachine, State>::TransitionBuilder<Args...>::~TransitionBuilder()
{
    machine_.addTransition(current_, next_, event_, condition_, action_, internal_);
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
StateMachine<ConcreteMachine, State>::TransitionBuilder<Args...>::TransitionBuilder(
    StateMachine& machine, State current, State next, EventFunc<Args...> event, bool internal)
  : machine_(machine), current_(current), next_(next), event_(event), condition_(), action_(), internal_(internal)
{
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
auto StateMachine<ConcreteMachine, State>::onTransition(State current, State next, EventFunc<Args...> event) -> TransitionBuilder<Args...>
{
    return TransitionBuilder<Args...>(*this, current, next, event, false);
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
auto StateMachine<ConcreteMachine, State>::onTransition(State current, EventFunc<Args...> event) -> TransitionBuilder<Args...>
{
    return TransitionBuilder<Args...>(*this, current, current, event, true);
}

template <typename ConcreteMachine, typename State>
template<typename Action>
void StateMachine<ConcreteMachine, State>::EntryExitActionBuilder::invoke(Action&& action)
{
    action_ = action;
}

template <typename ConcreteMachine, typename State>
template<typename T, typename R>
void StateMachine<ConcreteMachine, State>::EntryExitActionBuilder::invoke(T& obj, R(T::*function)())
{
    action_ = [&obj, function](){ (obj.*function)(); };
}

template <typename ConcreteMachine, typename State>
template<typename T, typename R>
void StateMachine<ConcreteMachine, State>::EntryExitActionBuilder::invoke(const T& obj, R(T::*function)() const)
{
    action_ = [&obj, function](){ (obj.*function)(); };
}

template <typename ConcreteMachine, typename State>
StateMachine<ConcreteMachine, State>::EntryExitActionBuilder::~EntryExitActionBuilder()
{
    if (entry_)
    {
        machine_.addEntryAction(current_, action_);
    }
    else
    {
        machine_.addExitAction(current_, action_);
    }
}

template <typename ConcreteMachine, typename State>
StateMachine<ConcreteMachine, State>::EntryExitActionBuilder::EntryExitActionBuilder(StateMachine& machine, State current, bool entry)
  : machine_(machine), current_(current), entry_(entry)
{
}

template <typename ConcreteMachine, typename State>
auto StateMachine<ConcreteMachine, State>::onEntry(State state) -> EntryExitActionBuilder
{
    return EntryExitActionBuilder(*this, state, true);
}

template <typename ConcreteMachine, typename State>
auto StateMachine<ConcreteMachine, State>::onExit(State state) -> EntryExitActionBuilder
{
    return EntryExitActionBuilder(*this, state, false);
}

template <typename ConcreteMachine, typename State>
void StateMachine<ConcreteMachine, State>::setParent(State parent, const std::set<State>& children)
{
    for (auto c : children)
    {
        setParent(parent, c);
    }
}

template <typename ConcreteMachine, typename State>
void StateMachine<ConcreteMachine, State>::setParent(State parent, State child)
{
    if (parent == child)
    {
        std::cerr << "Cannot set self as parent for state " << child << std::endl;
        return;
    }

    const auto ancestors = getAncestors(parent);
    for (auto a : ancestors)
    {
        if (a == child)
        {
            std::cerr << "Cannot create cyclic parent hierarchy for state " << child << " by setting " << parent << " as parent" << std::endl;
            return;
        }
    }

    if (parent_.find(child) != parent_.end())
    {
        std::cerr << "Cannot set parent " << parent << " for state " << child << " as it already has parent " << parent_[child] << std::endl;
        return;
    }

    parent_[child] = parent;
}

template <typename ConcreteMachine, typename State>
State StateMachine<ConcreteMachine, State>::getState() const
{
    return state_;
}

template <typename ConcreteMachine, typename State>
template<typename ...Args>
void StateMachine<ConcreteMachine, State>::addTransition(
    State current, State next, EventFunc<Args...> event,
    StoredFunction condition, StoredFunction action,
    bool internal)
{
    if (initialEntryExecuted_)
    {
        std::cerr << "Trying to add transitions after initial state entered" << std::endl;
        return;
    }

    transitions_.emplace(std::make_pair(
        Transition::createIdentifier(current, event),
        Transition(current, next, event, condition, action, internal)));
}

template <typename ConcreteMachine, typename State>
void StateMachine<ConcreteMachine, State>::addEntryAction(State state, std::function<void()> action)
{
    if (initialEntryExecuted_)
    {
        std::cerr << "Trying to add entry action after initial state entered" << std::endl;
        return;
    }

    if (!entryActions_.emplace(state, action).second)
    {
        std::cerr << "duplicate entry action for state " << state << std::endl;
    }
}

template <typename ConcreteMachine, typename State>
void StateMachine<ConcreteMachine, State>::addExitAction(State state, std::function<void()> action)
{
    if (initialEntryExecuted_)
    {
        std::cerr << "Trying to add exit action after initial state entered" << std::endl;
        return;
    }

    if (!exitActions_.emplace(state, action).second)
    {
        std::cerr << "duplicate exit action for state " << state << std::endl;
    }
}

template <typename ConcreteMachine, typename State>
template<typename ...Args, typename ...Args2>
void StateMachine<ConcreteMachine, State>::execute(EventFunc<Args...> event, Args2&&... args)
{
    auto transitionIt = findTransition(event, std::forward<Args2>(args)...);
    if (transitionIt == transitions_.end())
    {
        // Unhandled event
        std::cerr << "Unhandled event " << typeid(event).name() << " in state " << state_ << std::endl;
        --eventCount_;
        return;
    }

    const Transition& transition = transitionIt->second;
    const bool stateChanges = transition.changesState();
    const State previousState = state_;
    const State nextState = transition.getNextState();

    if (stateChanges)
    {
        const auto oldAncestors = getAncestorsUntilCommonAncestor(state_, nextState);
        for (auto a : oldAncestors)
        {
            exit(a);
        }
    }

    state_ = transition.execute(event, std::forward<Args2>(args)...);

    if (stateChanges)
    {
        const auto newAncestors = getAncestorsUntilCommonAncestor(state_, previousState);
        for (auto it = newAncestors.rbegin(); it != newAncestors.rend(); ++it)
        {
            enter(*it);
        }
    }
    --eventCount_;
}

template <typename ConcreteMachine, typename State>
void StateMachine<ConcreteMachine, State>::enter(State state)
{
    auto it = entryActions_.find(state);
    if (it != entryActions_.end())
    {
        it->second();
    }
}

template <typename ConcreteMachine, typename State>
void StateMachine<ConcreteMachine, State>::exit(State state)
{
    auto it = exitActions_.find(state);
    if (it != exitActions_.end())
    {
        it->second();
    }
}


template <typename ConcreteMachine, typename State>
std::vector<State> StateMachine<ConcreteMachine, State>::getAncestors(State state) const
{
    std::vector<State> ancestors;
    ancestors.push_back(state);

    auto it = parent_.find(state);
    while (it != parent_.end())
    {
        ancestors.push_back(it->second);
        it = parent_.find(it->second);
    }

    return ancestors;
}

template <typename ConcreteMachine, typename State>
std::vector<State> StateMachine<ConcreteMachine, State>::getAncestorsUntilCommonAncestor(State state, State referenceState) const
{
    const auto allAncestors = getAncestors(state);
    const auto allReferenceAncestors = getAncestors(referenceState);

    std::vector<State> ancestors;

    if (state == referenceState)
    {
        ancestors.push_back(state);
    }
    else
    {
        for (auto a : allAncestors)
        {
            for (auto ra : allReferenceAncestors)
            {
                if (a == ra)
                {
                    return ancestors;
                }
            }
            ancestors.push_back(a);
        }
    }

    return ancestors;
}


// Machine::Transition
template <typename ConcreteMachine, typename State>
template <typename ...Args>
StateMachine<ConcreteMachine, State>::Transition::Transition(
        State current, State next, EventFunc<Args...> event,
        StoredFunction condition, StoredFunction action,
        bool isInternal)
  : current_(current), next_(next), event_(identify(event)),
    action_(action),
    condition_(condition),
    internal_(isInternal)
{
}

template <typename ConcreteMachine, typename State>
bool StateMachine<ConcreteMachine, State>::Transition::changesState() const
{
    return !internal_;
}

template <typename ConcreteMachine, typename State>
State StateMachine<ConcreteMachine, State>::Transition::getNextState() const
{
    return next_;
}

template <typename ConcreteMachine, typename State>
template<typename ...Args, typename ...Args2>
bool StateMachine<ConcreteMachine, State>::Transition::checkCondition(EventFunc<Args...>, Args2&&... args) const
{
    if (!condition_)
    {
        return true;
    }

    auto& condition = *std::static_pointer_cast<ConditionFunc<Args...>>(condition_);
    if (condition)
    {
        return condition(std::forward<Args2>(args)...);
    }
    return true;
}

template <typename ConcreteMachine, typename State>
template<typename ...Args, typename ...Args2>
State StateMachine<ConcreteMachine, State>::Transition::execute(EventFunc<Args...>, Args2&&... args) const
{
    if (action_)
    {
        auto& action = *std::static_pointer_cast<ActionFunc<Args...>>(action_);
        if (action)
        {
            action(std::forward<Args2>(args)...);
        }
    }

    return next_;
}

template <typename ConcreteMachine, typename State>
auto StateMachine<ConcreteMachine, State>::Transition::getIdentifier() const -> Id
{
    return std::make_pair(current_, event_);
}

template <typename ConcreteMachine, typename State>
template <typename ...Args>
auto StateMachine<ConcreteMachine, State>::Transition::createIdentifier(State state, EventFunc<Args...> event) -> Id
{
    return std::make_pair(state, identify(event));
}

template <typename ConcreteMachine, typename State>
bool StateMachine<ConcreteMachine, State>::Transition::operator<(const Transition& o) const
{
    if (current_ < o.current_)
    {
        return true;
    }
    else if (current_ == o.current_)
    {
        return event_ < o.event_;
    }
    return false;
}

template <typename ConcreteMachine, typename State>
template <typename ...Args>
int StateMachine<ConcreteMachine, State>::Transition::identify(EventFunc<Args...> event)
{
    static std::vector<std::pair<EventFunc<Args...>, int>> eventFunctions;
    for (auto& f : eventFunctions)
    {
        if (f.first == event)
        {
            return f.second;
        }
    }

    const int idx = getEventIndex();
    eventFunctions.push_back(std::make_pair(event, idx));
    return idx;
}

template <typename ConcreteMachine, typename State>
int StateMachine<ConcreteMachine, State>::Transition::getEventIndex()
{
    static int idx = 0;
    return idx++;
}


template <typename ConcreteMachine, typename State>
template<typename ...Args, typename ...Args2>
auto StateMachine<ConcreteMachine, State>::findTransition(EventFunc<Args...> event, Args2&&... args) -> typename TransitionContainer::iterator
{
    for (auto a : getAncestors(state_))
    {
        auto range = transitions_.equal_range(Transition::createIdentifier(a, event));
        auto it = range.first;
        for (; it != range.second; ++it)
        {
            if (it->second.checkCondition(event, std::forward<Args2>(args)...))
            {
                return it;
            }
        }
    }

    return transitions_.end();
}

} // namespace Logic
