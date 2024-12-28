//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

//! [all]
#include <maki.hpp>
#include <iostream>

struct some_event{};
struct my_event{};
struct my_other_event{};

struct context{};

struct machine_conf_holder;
using machine_t = maki::machine<machine_conf_holder>;

//! [resolution]
using my_machine_ref_t = maki::machine_ref_e<my_event, my_other_event>;

constexpr auto my_action = maki::action_m([](const my_machine_ref_t mach)
{
    mach.process_event(my_event{});
});

constexpr auto my_other_action = maki::action_m([](const my_machine_ref_t mach)
{
    mach.process_event(my_other_event{});
});
//! [resolution]

constexpr auto my_state = maki::state_conf{};

constexpr auto my_other_state = maki::state_conf{};

constexpr auto transition_table = maki::transition_table{}
    (my_state, maki::type<some_event>,     maki::null, my_action)
    (my_state, maki::type<my_event>,       maki::null, my_other_action)
    (my_state, maki::type<my_other_event>, my_other_state)
;

struct machine_conf_holder
{
    static constexpr auto conf = maki::machine_conf{}
        .context_a(maki::type<context>)
        .transition_tables(transition_table)
    ;
};

int main()
{
    auto machine = machine_t{};
    machine.process_event(some_event{});

    if(machine.is<my_other_state>())
    {
        std::cout << "OK\n";
    }
    else
    {
        std::cout << "NOK\n";
    }
}
//! [all]
