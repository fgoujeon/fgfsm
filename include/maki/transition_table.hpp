//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the maki::transition and maki::transition_table struct templates
*/

#ifndef MAKI_TRANSITION_TABLE_HPP
#define MAKI_TRANSITION_TABLE_HPP

#include "null.hpp"
#include "detail/storable_function.hpp"
#include "detail/tuple.hpp"
#include "detail/tu.hpp"

namespace maki
{

/**
@defgroup TransitionTable Transition Table
@brief These are the types and functions that must be used to define transition tables.

A transition table lists all the possible transitions from a state (the source
state) to another (the target state) in a region of a state machine.

You can define a transition table by using this class template directly:
```cpp
constexpr auto transition_table = maki::transition_table
<
    maki::transition<off, button_press, on,  turn_light_on, has_enough_power>,
    maki::transition<on,  button_press, off, turn_light_off>
>{};
```

… but using the @ref transition_table variable template and the
@ref transition_table::add_c member type template is usually the preferred,
more concise way to do so:
```cpp
constexpr auto transition_table = maki::transition_table{}
    .add_c<off, button_press, on,  turn_light_on, has_enough_power>
    .add_c<on,  button_press, off, turn_light_off>
;
```

Note that the first usage may be more appropriate in the context of a template
in order to avoid awkward `typename`s and `template`s.

@{
*/

template<class... Transitions>
class transition_table;

namespace detail
{
    /**
    @brief Represents a possible state transition.

    Used as a template argument of @ref transition_table.

    @tparam SourceStatePattern the active state (or states, plural, if it's a @ref TypePatterns "type pattern") from which the transition can occur
    @tparam EventPattern the event type (or types, plural, if it's a @ref TypePatterns "type pattern") that can cause the transition to occur
    @tparam TargetState the state that becomes active after the transition occurs
    @tparam Action the function invoked when the transition occurs
    @tparam Guard the function that must return `true` for the transition to occur
    */
    template
    <
        class SourceStateConfPatternPtr,
        class EventPattern,
        class TargetStateConfPtr,
        class Action,
        class Guard
    >
    struct transition
    {
        SourceStateConfPatternPtr psource_state_conf_pattern;
        EventPattern event_pattern;
        TargetStateConfPtr ptarget_state_conf;
        Action action;
        Guard guard;
    };

    template
    <
        class SourceStateConfPatternPtr,
        class EventPattern,
        class TargetStateConfPtr,
        class Action,
        class Guard
    >
    constexpr auto make_transition
    (
        const SourceStateConfPatternPtr psource_state_conf_pattern,
        const EventPattern& event_pattern,
        const TargetStateConfPtr ptarget_state_conf,
        const Action& action,
        const Guard& guard
    )
    {
        return transition
        <
            SourceStateConfPatternPtr,
            EventPattern,
            TargetStateConfPtr,
            storable_function_t<Action>,
            storable_function_t<Guard>
        >
        {
            psource_state_conf_pattern,
            event_pattern,
            ptarget_state_conf,
            action,
            guard
        };
    }

    template<class... Transitions>
    constexpr auto make_transition_table(const Transitions&... transitions)
    {
        return transition_table<Transitions...>{transitions...};
    }

    template<class... Transitions>
    constexpr const auto& rows(const transition_table<Transitions...>& table)
    {
        return table.transitions_;
    }
}

/**
@brief Represents a transition table.

@tparam Transitions the transitions, which must be instances of @ref transition
*/
template<class... Transitions>
class transition_table
{
public:
    constexpr transition_table() = default;

    constexpr transition_table(const transition_table&) = default;

    transition_table(transition_table&&) = delete;

    ~transition_table() = default;

    transition_table& operator=(const transition_table&) = delete;

    transition_table& operator=(transition_table&&) = delete;

    template
    <
        class SourceStateConfPattern,
        class EventPattern,
        class TargetStateConf,
        class Action = null_t,
        class Guard = null_t
    >
    constexpr auto operator()
    (
        const SourceStateConfPattern& source_state_conf_pattern,
        const EventPattern& event_pattern,
        const TargetStateConf& target_state_conf,
        const Action& action = null,
        const Guard& guard = null
    )
    {
        return detail::tu::apply
        (
            transitions_,
            []
            (
                const SourceStateConfPattern& source_state_conf_pattern,
                const EventPattern& event_pattern,
                const TargetStateConf& target_state_conf,
                const Action& action,
                const Guard& guard,
                const auto&... transitions
            )
            {
                return detail::make_transition_table
                (
                    transitions...,
                    detail::make_transition
                    (
                        &source_state_conf_pattern,
                        event_pattern,
                        &target_state_conf,
                        action,
                        guard
                    )
                );
            },
            source_state_conf_pattern,
            event_pattern,
            target_state_conf,
            action,
            guard
        );
    }

private:
    template<class... Transitions2>
    friend constexpr auto detail::make_transition_table(const Transitions2&...);

    template<class... Transitions2>
    friend constexpr const auto& detail::rows(const transition_table<Transitions2...>&);

    template<class... Transitions2>
    constexpr explicit transition_table(const Transitions&... transitions):
        transitions_{detail::distributed_construct, transitions...}
    {
    }

    detail::tuple<Transitions...> transitions_;
};

/**
@}
*/

} //namespace

#endif
