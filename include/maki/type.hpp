//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the maki::t struct template
*/

#ifndef MAKI_TYPE_HPP
#define MAKI_TYPE_HPP

#include <type_traits>

namespace maki
{

/**
@brief A type holder
*/
template<class T>
struct type_t
{
    /**
    @brief An alias for the given type
    */
    using type = T;
};

/**
@related type
@brief Returns whether T and U are the same type
*/
template<class T, class U>
constexpr bool operator==(const type_t<T> /*lhs*/, const type_t<U> /*rhs*/)
{
    return std::is_same_v<T, U>;
}

/**
@related type
@brief A convenient variable template for @ref type
*/
template<class T>
constexpr auto type = type_t<T>{};

} //namespace

#endif
