//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_STATE_TRAITS_HPP
#define MAKI_DETAIL_STATE_TRAITS_HPP

#include "simple_state_no_context_fwd.hpp"
#include "simple_state_fwd.hpp"
#include "submachine_no_context_fwd.hpp"
#include "submachine_fwd.hpp"
#include "conf_traits.hpp"
#include "state_id_traits.hpp"
#include "tlu.hpp"
#include <type_traits>

namespace maki::detail::state_traits
{

//state_id_to_state

template<auto StateId, const auto& ParentPath, bool IsSubmachine, bool HasContext>
struct state_id_to_state_impl;

template<auto StateId, const auto& ParentPath>
struct state_id_to_state_impl<StateId, ParentPath, false, false>
{
    using type = simple_state_no_context<StateId>;
};

template<auto StateId, const auto& ParentPath>
struct state_id_to_state_impl<StateId, ParentPath, false, true>
{
    using type = simple_state<StateId>;
};

template<auto StateId, const auto& ParentPath>
struct state_id_to_state_impl<StateId, ParentPath, true, false>
{
    static constexpr auto path = ParentPath.template add_state<*StateId>();
    using type = submachine_no_context<StateId, path>;
};

template<auto StateId, const auto& ParentPath>
struct state_id_to_state_impl<StateId, ParentPath, true, true>
{
    static constexpr auto path = ParentPath.template add_state<*StateId>();
    using type = submachine<StateId, path>;
};

template<auto StateId, const auto& ParentPath>
struct state_id_to_state
{
    using type = typename state_id_to_state_impl
    <
        StateId,
        ParentPath,
        conf_traits::is_submachine_conf_v<std::decay_t<decltype(*StateId)>>,
        state_id_traits::has_context_v<StateId>
    >::type;
};

template<auto StateId, const auto& ParentPath>
using state_id_to_state_t = typename state_id_to_state
<
    StateId,
    ParentPath
>::type;


//has_id

template<auto Id>
struct for_id
{
    template<class T>
    struct has_id
    {
        static constexpr auto value = static_cast<const void*>(T::identifier) == static_cast<const void*>(Id);
    };
};

} //namespace

#endif
