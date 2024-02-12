//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the maki::submachine_conf struct template
*/

#ifndef MAKI_SUBMACHINE_CONF_FWD_HPP
#define MAKI_SUBMACHINE_CONF_FWD_HPP

#include "detail/context_signature.hpp"
#include "detail/tuple.hpp"
#include <string_view>

namespace maki
{

namespace detail
{
    template
    <
        class Context = void,
        class EntryActionTuple = tuple<>,
        class InternalActionTuple = tuple<>,
        class ExitActionTuple = tuple<>,
        class TransitionTableFnTuple = tuple<>
    >
    struct submachine_conf_option_set
    {
        using context_type = Context;

        context_signature context_sig = context_signature::v;
        EntryActionTuple entry_actions;
        InternalActionTuple internal_actions;
        ExitActionTuple exit_actions;
        std::string_view pretty_name;
        TransitionTableFnTuple transition_table_fns;
    };
}

#ifdef MAKI_DETAIL_DOXYGEN
/**
@brief Submachine configuration
*/
template<IMPLEMENTATION_DETAIL>
#else
template<class OptionSet = detail::submachine_conf_option_set<>>
#endif
class submachine_conf;

} //namespace

#endif
