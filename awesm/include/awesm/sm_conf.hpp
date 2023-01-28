//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SM_CONF_HPP
#define AWESM_SM_CONF_HPP

#include "transition_table_list.hpp"
#include "pretty_name.hpp"

namespace awesm
{

namespace sm_options
{
    struct after_state_transition{};
    struct before_entry{};
    struct before_state_transition{};
    struct disable_run_to_completion{};
    struct on_event{};
    struct on_exception{};

    template<size_t MaxSize, size_t MaxAlignmentRequirement>
    struct small_event_requirements
    {
        static constexpr size_t get_small_event_max_size()
        {
            return MaxSize;
        }

        static constexpr size_t get_small_event_max_alignment_requirement()
        {
            return MaxAlignmentRequirement;
        }
    };

    using get_pretty_name = detail::get_pretty_name_option;
}

template<class TransitionTable, class Context, class... Options>
struct sm_conf
{
    using transition_table_list_type = transition_table_list<TransitionTable>;
    using context_type = Context;
    static constexpr auto region_count = 1;
};

template<class... TransitionTables, class Context, class... Options>
struct sm_conf<transition_table_list<TransitionTables...>, Context, Options...>
{
    using transition_table_list_type = transition_table_list<TransitionTables...>;
    using context_type = Context;
    static constexpr auto region_count = sizeof...(TransitionTables);
};

} //namespace

#endif
