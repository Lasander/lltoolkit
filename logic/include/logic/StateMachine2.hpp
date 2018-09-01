#pragma once

#include <functional>
#include <iostream>
#include <queue>
#include <tuple>
#include <vector>

namespace Logic {

/**
 * State machine implementation.
 *
 * @tparam State Type defining state identity. Should be light-weight to copy, e.g. enum.
 *
 * TODO:
 *  - entry/exit actions
 *  - handle internal transition (no entry/exit)
 *  - state hierarchy
 *
 */
template <typename State>
class StateMachine
{
public:
    /**
     * Construct a StateMachine with @p initialState
     */
    StateMachine(State initialState)
      : state_{initialState},
        eventCount_{0},
        events_{}
    {
    }

    /**
     * State machine event
     *
     * Events should be shared with state machine client and invoked with call operator, i.e.:
     *   event(param1, param2);
     *
     * @tparam Args event argument types
     */
    template <typename ...Args>
    class Event
    {
        using Condition = std::function<bool(Args...)>;
        using Action = std::function<void(Args...)>;

        struct Transition
        {
            State next_;
            bool internal_{false};
            Action action_{};
            Condition condition_{};
        };

        using TransitionContainer = std::vector<std::pair<State, Transition>>;
        TransitionContainer transitions_;

        // Associated machine, used for convenience operator()
        StateMachine& machine_;
        // Event name, used for debug and error logging
        std::string name_;

        // Allow Machine to find transition
        friend class StateMachine;

        /**
         * Find transition from @p state with this event.
         *
         * If there are multiple transactions with this event, their conditions are evaluated
         * in registration order, and first matching is returned.
         *
         * @param args event arguments, passed to possible transition conditions
         * @return iterator pointing to found transition or transitions.end() if none found
         */
        template <typename... ActualArgs>
        typename TransitionContainer::iterator findTransition(State state, ActualArgs&&... args)
        {
            for (auto it = transitions_.begin(); it != transitions_.end(); ++it)
            {
                if (it->first == state)
                {
                    auto& transition = it->second;
                    if (transition.condition_ && !transition.condition_(std::forward<ActualArgs>(args)...))
                    {
                        continue;
                    }
                    return it;
                }
            }
            return transitions_.end();
        }

        template<typename ...ActualArgs>
        void execute(ActualArgs&&... args)
        {
            const auto state = machine_.state_;

            auto it = findTransition(state, std::forward<ActualArgs>(args)...);
            if (it != transitions_.end())
            {
                machine_.state_ = it->second.next_;
                if (it->second.action_)
                {
                    it->second.action_(std::forward<ActualArgs>(args)...);
                }
            }
            else
            {
                std::cerr << "Unexpected event " << name_ << " in state " << state << std::endl;
            }
        }

    public:
        /**
         * Create state machine event for @p machine machine with @p name
         */
        Event(StateMachine& machine, std::string name) : machine_{machine}, name_{std::move(name)}
        {
        }

        /**
         * Call operator to trigger the event with @p args
         */
        template<typename ...ActualArgs>
        void operator()(ActualArgs&&... args)
        {
            machine_.handle(*this, std::forward<ActualArgs>(args)...);
        }

        /**
         * Transition builder used to add actions and conditions to transactions.
         *
         * @see StateMachine::add
         */
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
            template <typename Condition>
            TransitionBuilder& when(Condition&& action)
            {
                transition_.condition_ = std::forward<Condition>(action);
                return *this;
            }

            /**
            * @name Add transition action
            *
            * Action must:
            * - be a callable with the event parameters
            *
            * Note: any return value from action is ignored.
            */
            template <typename Action>
            void invoke(Action&& action)
            {
                transition_.action_ = std::forward<Action>(action);
            }

            TransitionBuilder(const TransitionBuilder&) = delete;
            TransitionBuilder& operator=(const TransitionBuilder&) = delete;
            ~TransitionBuilder()
            {
                event_.transitions_.emplace_back(current_, std::move(transition_));
            }

        private:
            // Allow Machine to create and move
            friend class StateMachine;

            /**
             * Create transition builder. Destroying the builder will add the transition to the machine.
             *
             * @param current state where the transition starts
             * @param next state where the transition leads, can be same as current
             * @param event event triggering the transition
             * @param internal true if this is a state internal event (no re-entry), this also implies that current == next
             */
            TransitionBuilder(State current, State next, Event<Args...>& event, bool internal)
              : event_{event},
                current_{current},
                transition_{next, internal}
            {
            }
            TransitionBuilder(TransitionBuilder&&) = default;

        private:
            Event<Args...>& event_;
            State current_;
            typename Event<Args...>::Transition transition_;
        };
    };

    /**
     * Add transition from @p state to @p nextState with @p event
     *
     * @return TransitionBuilder that can be used to add optional elements to the transition.
     *
     * @example
     *   Machine::Event<int> event;
     *   machine.add(STATE1, event, STATE2).invoke([]{ doSomething(); });
     */
    template <typename ...Args>
    auto add(State state, Event<Args...>& event, State nextState) const
    {
        return typename Event<Args...>::TransitionBuilder(state, nextState, event, false);
    }

    /**
     * Add internal transition within @p state with @p event
     *
     * @return TransitionBuilder that can be used to add optional elements to the transition.
     *
     * @example
     *   Machine::Event<int> event;
     *   machine.add(STATE1, event).invoke([]{ doSomething(); });
     */
    template <typename ...Args>
    auto add(State state, Event<Args...>& event) const
    {
        return typename Event<Args...>::TransitionBuilder(state, state, event, true);
    }

private:
    /**
     * Handle @p event with @p args.
     *
     * In case of recursive call to this machine (e.g. event action invoking new event),
     * queue the new events to be executed in order after the ongoing event.
     */
    template<typename ...Args, typename ...ActualArgs>
    void handle(Event<Args...>& event, ActualArgs&&... args)
    {
        if (++eventCount_ > 1)
        {
            // Queue recursive events to be handled after current event
            events_.emplace([event, args...]() mutable { event.execute(args...); });
            return;
        }

        // Handle primary event
        event.execute(std::forward<ActualArgs>(args)...);
        --eventCount_;

        // Handle queued events
        while (!events_.empty())
        {
            events_.front()();
            --eventCount_;
            events_.pop();
        }

        assert(eventCount_ == 0);
    }

    // Current machine state
    State state_;

    // Queued events
    int eventCount_;
    std::queue<std::function<void()>> events_;
};

} // namespace Logic
