//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SM_OPTIONS_HPP
#define AWESM_SM_OPTIONS_HPP

#include "sm_options/before_entry.hpp"
#include "sm_options/in_state_internal_transitions.hpp"
#include "sm_options/on_event.hpp"
#include "sm_options/on_exception.hpp"
#include "sm_options/run_to_completion.hpp"

namespace awesm::sm_options
{

struct after_state_transition
{
    //Dummy
    template<class Sm, class Context>
    after_state_transition(Sm& /*sm*/, Context& /*ctx*/)
    {
    }

    //Dummy
    void call()
    {
    }
};

struct before_state_transition
{
    //Dummy
    template<class Sm, class Context>
    before_state_transition(Sm& /*sm*/, Context& /*ctx*/)
    {
    }

    //Dummy
    void call()
    {
    }
};

} //namespace

#endif
