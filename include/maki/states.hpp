//Copyright Florian Goujeon 2021 - 2025.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_STATES_HPP
#define MAKI_STATES_HPP

#include "detail/state_impls/simple_no_context.hpp"
#include "state.hpp"
#include "state_conf.hpp"

namespace maki
{

namespace detail::state_confs
{
    inline constexpr auto init = state_conf{};
    inline constexpr auto fin = state_conf{};
}

/**
@brief Predefined state and pseudostate objects.
*/
namespace states
{
#if MAKI_DETAIL_DOXYGEN
    /**
    @brief Dummy state object given to transition hooks for transitions from initial pseudostate.
    */
    constexpr auto init = state<IMPLEMENTATION_DETAIL>{};
#else
    inline constexpr auto init = state<detail::state_impls::simple_no_context<&detail::state_confs::init>>{};
#endif

#if MAKI_DETAIL_DOXYGEN
    /**
    @brief Dummy state object given to transition hooks for transitions to final state.
    */
    constexpr auto fin = state<IMPLEMENTATION_DETAIL>{};
#else
    inline constexpr auto fin = state<detail::state_impls::simple_no_context<&detail::state_confs::fin>>{};
#endif
}

} //namespace

#endif
