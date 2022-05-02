//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#ifndef FGFSM_STATE_TRANSITION_POLICY_HELPER_HPP
#define FGFSM_STATE_TRANSITION_POLICY_HELPER_HPP

#include "none.hpp"
#include "detail/tlu.hpp"
#include "detail/call_member.hpp"
#include <type_traits>
#include <cassert>

namespace fgfsm
{

template
<
    class Fsm,
    class StartState,
    class Event,
    class TargetState,
    class Action,
    class Guard
>
class state_transition_policy_helper
{
    private:
        template<class FsmConfiguration>
        friend class fsm;

        state_transition_policy_helper(Fsm& sm, const Event& event):
            sm_(sm),
            event_(event)
        {
        }

    public:
        bool check_guard() const
        {
            if constexpr(!std::is_same_v<Guard, none>)
            {
                using tag_t = Guard*;
                return detail::call_check
                (
                    sm_.guards_.get(tag_t{}),
                    event_
                );
            }
            else
            {
                return true;
            }
        }

        void invoke_start_state_on_exit() const
        {
            if constexpr(!std::is_same_v<TargetState, none>)
            {
                using tag_t = StartState*;
                detail::call_on_exit
                (
                    sm_.states_.get(tag_t{}),
                    event_
                );
            }
        }

        void activate_target_state() const
        {
            if constexpr(!std::is_same_v<TargetState, none>)
            {
                sm_.active_state_index_ = detail::tlu::get_index
                <
                    typename Fsm::state_tuple,
                    TargetState
                >;
            }
        }

        void execute_action() const
        {
            if constexpr(!std::is_same_v<Action, none>)
            {
                using tag_t = Action*;
                detail::call_execute
                (
                    sm_.actions_.get(tag_t{}),
                    event_
                );
            }
        }

        void invoke_target_state_on_entry() const
        {
            if constexpr(!std::is_same_v<TargetState, none>)
            {
                using tag_t = TargetState*;
                detail::call_on_entry
                (
                    sm_.states_.get(tag_t{}),
                    event_
                );
            }
        }

    private:
        Fsm& sm_;
        const Event& event_;
};

} //namespace

#endif
