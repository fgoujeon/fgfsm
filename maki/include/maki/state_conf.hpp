//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_STATE_CONF_HPP
#define MAKI_STATE_CONF_HPP

#include "detail/tlu.hpp"
#include "detail/type_list.hpp"
#include "detail/type.hpp"

namespace maki
{

namespace detail
{
    template<class... Args>
    constexpr auto make_state_conf(const Args&... args);
}

template<class OnEventTypeList = detail::type_list<>>
struct state_conf
{
    bool on_entry_any = false; //NOLINT(misc-non-private-member-variables-in-classes)
    bool on_event_auto = false; //NOLINT(misc-non-private-member-variables-in-classes)
    OnEventTypeList on_event_types; //NOLINT(misc-non-private-member-variables-in-classes)
    bool on_exit_any = false; //NOLINT(misc-non-private-member-variables-in-classes)
    bool pretty_name_fn = false; //NOLINT(misc-non-private-member-variables-in-classes)

    [[nodiscard]] constexpr auto enable_on_entry_any() const
    {
        return detail::make_state_conf
        (
            true,
            on_event_auto,
            on_event_types,
            on_exit_any,
            pretty_name_fn
        );
    }

    [[nodiscard]] constexpr auto set_on_event_types_auto() const
    {
        return detail::make_state_conf
        (
            on_entry_any,
            true,
            on_event_types,
            on_exit_any,
            pretty_name_fn
        );
    }

    template<class... Types>
    [[nodiscard]] constexpr auto set_on_event_types() const
    {
        return detail::make_state_conf
        (
            on_entry_any,
            on_event_auto,
            detail::type_list_c<Types...>,
            on_exit_any,
            pretty_name_fn
        );
    }

    template<class... Types>
    [[nodiscard]] constexpr auto set_on_event_types(const detail::type_list<Types...> value) const
    {
        return detail::make_state_conf
        (
            on_entry_any,
            on_event_auto,
            value,
            on_exit_any,
            pretty_name_fn
        );
    }

    [[nodiscard]] constexpr auto enable_on_exit_any() const
    {
        return detail::make_state_conf
        (
            on_entry_any,
            on_event_auto,
            on_event_types,
            true,
            pretty_name_fn
        );
    }

    [[nodiscard]] constexpr auto enable_pretty_name_fn() const
    {
        return detail::make_state_conf
        (
            on_entry_any,
            on_event_auto,
            on_event_types,
            on_exit_any,
            true
        );
    }
};

inline constexpr auto state_conf_c = state_conf<>{};

namespace detail
{
    template<class... Args>
    constexpr auto make_state_conf(const Args&... args)
    {
        using args_t = detail::type_list<Args...>;
        using on_event_type_list = tlu::get_t<args_t, 2>;
        return state_conf<on_event_type_list>{args...};
    }
}

} //namespace

#endif
