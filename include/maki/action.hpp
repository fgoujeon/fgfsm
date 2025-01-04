//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the maki::action class template
*/

#ifndef MAKI_ACTION_HPP
#define MAKI_ACTION_HPP

#include "detail/signature_macros.hpp"
#include "detail/call.hpp"

namespace maki
{

/**
@brief The set of arguments taken by an action callable.

Meaning of every letter:

- `v`: void
- `m`: machine
- `c`: context
- `e`: event
*/
enum class action_signature: char
{
#define MAKI_DETAIL_X(name) name, /*NOLINT(cppcoreguidelines-macro-usage)*/
    MAKI_DETAIL_ACTION_SIGNATURES
#undef MAKI_DETAIL_X
};

/**
@brief Represents an action to be given to `maki::transition_table`.
Use the builder functions (`maki::action_v()` and its variants) instead of using
this type directly.
*/
template<action_signature Sig, class Callable>
struct action
{
    static constexpr action_signature signature = Sig;
    Callable callable;
};

#define MAKI_DETAIL_X(name) /*NOLINT(cppcoreguidelines-macro-usage)*/ \
    /** \
    @brief Makes a `maki::action` with the indicated signature and given  \
    callable. \
    */ \
    template<class Callable> \
    constexpr action<action_signature::name, Callable> action_##name(const Callable& callable) \
    { \
        return {callable}; \
    }
MAKI_DETAIL_ACTION_SIGNATURES
#undef MAKI_DETAIL_X

namespace detail
{
    template
    <
        class Action,
        class Context,
        class Machine,
        class Event
    >
    void call_action
    (
        const Action& act,
        Context& ctx,
        Machine& mach,
        const Event& event
    )
    {
        return call_callable<action_signature, Action::signature>
        (
            act.callable,
            ctx,
            mach,
            event
        );
    }
}

} //namespace

#endif
