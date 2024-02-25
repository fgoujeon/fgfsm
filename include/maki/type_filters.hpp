//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the type filter struct templates
*/

#ifndef MAKI_TYPE_FILTERS_HPP
#define MAKI_TYPE_FILTERS_HPP

#include "detail/storable_conf_filter.hpp"
#include "constant.hpp"
#include "type.hpp"
#include <type_traits>

namespace maki
{

/**
@defgroup TypeFilters Type Filters
@brief Type filters can be used in some places of the API (such as in transition
tables) in lieu of single types to concisely express a set of types.

@{
*/

/**
@brief A type filter that matches with any type.
*/
struct any_t{};

/**
@brief A type filter that matches with any type that verifies `Predicate<T>::value == true`.
@tparam Predicate the predicate against which types are tested
*/
template<class Predicate>
struct any_if_t
{
    Predicate pred;
};

/**
@brief A type filter that matches with any type that verifies `Predicate<T>::value == false`.
@tparam Predicate the predicate against which types are tested
*/
template<class Predicate>
struct any_if_not_t
{
    Predicate pred;
};

/**
@brief A type filter that matches with the given types.
@tparam Ts the types the filter matches with
*/
template<class... Ts>
struct any_of_t
{
    detail::tuple<Ts...> values;
};

/**
@brief A type filter that matches with any type but the given ones.
@tparam Ts the types the filter doesn't match with
*/
template<class... Ts>
struct any_but_t
{
    detail::tuple<Ts...> values;
};

/**
@brief A type filter that doesn't match with any type.
*/
struct none_t{};

/**
@}
*/

inline constexpr auto any = any_t{};

template<class Predicate>
constexpr auto any_if(const Predicate& pred)
{
    return any_if_t<Predicate>{pred};
}

template<class Predicate>
constexpr auto any_if_not(const Predicate& pred)
{
    return any_if_not_t<Predicate>{pred};
}

template<class... Ts>
constexpr auto any_of(const Ts&... values)
{
    return any_of_t<detail::storable_conf_filter_t<Ts>...>
    {
        detail::tuple<detail::storable_conf_filter_t<Ts>...>
        {
            detail::distributed_construct,
            detail::to_storable_conf_filter(values)...
        }
    };
}

template<class... Ts>
constexpr auto any_but(const Ts&... values)
{
    return any_but_t<detail::storable_conf_filter_t<Ts>...>
    {
        detail::tuple<detail::storable_conf_filter_t<Ts>...>
        {
            detail::distributed_construct,
            detail::to_storable_conf_filter(values)...
        }
    };
}

inline constexpr auto none = none_t{};


template<class... Types>
constexpr auto any_type_of = any_of(type<Types>...);

template<class... Types>
constexpr auto any_type_but = any_but(type<Types>...);


//matches_filter
namespace detail
{
    //Filter is a regular type
    template<class Value, class Filter>
    constexpr bool matches_filter(const Value& value, const Filter& filter)
    {
        return value == filter;
    }

    template<class Value>
    constexpr bool matches_filter(const Value& /*value*/, const any_t /*filter*/)
    {
        return true;
    }

    template<class Value, class Predicate>
    constexpr bool matches_filter(const Value& value, const any_if_t<Predicate>& filter)
    {
        return filter.pred(value);
    }

    template<class Value, class Predicate>
    constexpr bool matches_filter(const Value& value, const any_if_not_t<Predicate>& filter)
    {
        return !filter.pred(value);
    }

    template<class Value, class... Ts>
    constexpr bool matches_filter(const Value& value, const any_of_t<Ts...>& filter)
    {
        return tuple_apply
        (
            filter.values,
            [](const Value& value, const Ts&... values)
            {
                return (matches_filter(value, values) || ...);
            },
            value
        );
    }

    template<class Value, class... Ts>
    constexpr bool matches_filter(const Value& value, const any_but_t<Ts...>& filter)
    {
        return tuple_apply
        (
            filter.values,
            [](const Value& value, const Ts&... values)
            {
                return (!matches_filter(value, values) && ...);
            },
            value
        );
    }

    template<class Value>
    constexpr bool matches_filter(const Value& /*value*/, const none_t /*filter*/)
    {
        return false;
    }
}

namespace detail
{
    template<class T>
    struct is_type_filter
    {
        static constexpr auto value = false;
    };

    template<class T>
    constexpr auto is_type_filter_v = is_type_filter<T>::value;

    template<>
    struct is_type_filter<any_t>
    {
        static constexpr auto value = true;
    };

    template<class Predicate>
    struct is_type_filter<any_if_t<Predicate>>
    {
        static constexpr auto value = true;
    };

    template<class Predicate>
    struct is_type_filter<any_if_not_t<Predicate>>
    {
        static constexpr auto value = true;
    };

    template<class... Ts>
    struct is_type_filter<any_of_t<Ts...>>
    {
        static constexpr auto value = true;
    };

    template<class... Ts>
    struct is_type_filter<any_but_t<Ts...>>
    {
        static constexpr auto value = true;
    };

    template<>
    struct is_type_filter<none_t>
    {
        static constexpr auto value = true;
    };

    template<class Value, class... Filters>
    constexpr bool matches_any_filter(const Value& value, const Filters&... filters)
    {
        return (matches_filter(value, filters) || ...);
    }
}

} //namespace

#endif
