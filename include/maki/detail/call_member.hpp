//Copyright Florian Goujeon 2021.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_without_event.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_CALL_MEMBER_HPP
#define MAKI_DETAIL_CALL_MEMBER_HPP

#include "type_traits.hpp"
#include "state_traits.hpp"
#include "event_action.hpp"
#include "tlu.hpp"
#include "tu.hpp"
#include "../type_patterns.hpp"
#include <functional>
#include <type_traits>
#include <utility>

namespace maki::detail
{

template<class Machine, class Context, class Event, class EventAction, class... EventActions>
void call_event_action
(
    [[maybe_unused]] Machine& mach,
    [[maybe_unused]] Context& ctx,
    [[maybe_unused]] const Event& event,
    [[maybe_unused]] const EventAction& event_action,
    [[maybe_unused]] const EventActions&... event_actions
)
{
    if constexpr(matches_pattern(type<Event>, typename EventAction::event_filter_type{}))
    {
        if constexpr(EventAction::sig == event_action_signature::v)
        {
            std::invoke(event_action.action);
        }
        else if constexpr(EventAction::sig == event_action_signature::c)
        {
            std::invoke(event_action.action, ctx);
        }
        else if constexpr(EventAction::sig == event_action_signature::cm)
        {
            std::invoke(event_action.action, ctx, mach);
        }
        else if constexpr(EventAction::sig == event_action_signature::ce)
        {
            std::invoke(event_action.action, ctx, event);
        }
        else if constexpr(EventAction::sig == event_action_signature::m)
        {
            std::invoke(event_action.action, mach);
        }
        else if constexpr(EventAction::sig == event_action_signature::me)
        {
            std::invoke(event_action.action, mach, event);
        }
        else if constexpr(EventAction::sig == event_action_signature::e)
        {
            std::invoke(event_action.action, event);
        }
        else
        {
            constexpr auto is_false = sizeof(Machine) == 0;
            static_assert(is_false, "Unsupported event_action_signature value");
        }
    }
    else
    {
        static_assert(sizeof...(EventActions) != 0, "No event action found for this state and event");
        call_event_action(mach, ctx, event, event_actions...);
    }
}

template
<
    class ActionTuple,
    class Machine,
    class Context,
    class Event
>
void call_state_action
(
    const ActionTuple& actions,
    Machine& mach,
    Context& ctx,
    const Event& event
)
{
    if constexpr(!tlu::empty_v<ActionTuple>)
    {
        tu::apply
        (
            actions,
            [](auto&... args)
            {
                call_event_action(args...);
            },
            mach,
            ctx,
            event
        );
    }
}

template<class Fn, class Machine, class Context, class Event>
auto call_action_or_guard
(
    const Fn& fun,
    [[maybe_unused]] Machine& mach,
    [[maybe_unused]] Context& ctx,
    [[maybe_unused]] const Event& event
)
{
    if constexpr(std::is_invocable_v<Fn, Context&, Machine&, const Event&>)
    {
        return std::invoke(fun, ctx, mach, event);
    }
    else if constexpr(std::is_invocable_v<Fn, Context&, const Event&>)
    {
        return std::invoke(fun, ctx, event);
    }
    else if constexpr(std::is_invocable_v<Fn, Context&>)
    {
        return std::invoke(fun, ctx);
    }
    else if constexpr(is_nullary_v<Fn>)
    {
        return std::invoke(fun);
    }
    else
    {
        constexpr auto is_false = sizeof(Machine) == 0;
        static_assert(is_false, "No valid signature found for action/guard");
    }
}

} //namespace

#endif
