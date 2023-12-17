//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

//I lied muahahahahaha!
#define main fake_main

#include <iostream>

//! [all]
#include "user_interface.hpp"
#include "motor.hpp"
#include <maki.hpp>

//The context, instantiated by machine_t and accessible to all the actions and guards
struct context
{
    /*
    Note: Machine is always machine_t (defined below).
    We need a template to break circular dependency.
    */
    template<class Machine>
    context(Machine& machine):
        user_itf(make_event_callback(machine)),
        mtr(make_event_callback(machine))
    {
    }

    /*
    Forward events (from event variants) to state machine.

    Note: Machine is always machine_t (defined below).
    We need a template to break circular dependency.
    */
    template<class Machine>
    static auto make_event_callback(Machine& machine)
    {
        return [&machine](const auto& event_variant)
        {
            std::visit
            (
                [&machine](const auto& event)
                {
                    machine.process_event(event);
                },
                event_variant
            );
        };
    }

    user_interface user_itf;
    motor mtr;
};

//States
constexpr auto idle = maki::state_conf_c;
constexpr auto starting = maki::state_conf_c;
constexpr auto running = maki::state_conf_c;
constexpr auto stopping = maki::state_conf_c;

//Actions
void start_motor(context& ctx)
{
    ctx.mtr.async_start();
}
void stop_motor(context& ctx)
{
    ctx.mtr.async_stop();
}

//Transition table
constexpr auto transition_table = maki::transition_table_c
    //     source state, event,                         target state, action
    .add_c<idle,         user_interface::start_request, starting,     start_motor>
    .add_c<starting,     motor::start_event,            running>
    .add_c<running,      user_interface::stop_request,  stopping,     stop_motor>
    .add_c<stopping,     motor::stop_event,             idle>
;

//State machine configuration
struct machine_conf_holder
{
    static constexpr auto conf = maki::machine_conf_c
        .transition_tables(transition_table)
        .context<context>()
    ;
};

//State machine
using machine_t = maki::machine<machine_conf_holder>;

int main()
{
    //Instantiate the state machine
    auto machine = machine_t{};

    //Run indefinitely...
    //[Implementation detail...]

    return 0;
}
//! [all]

#undef main

//We want to test this program without spamming the doc with testing details.
int main()
{
    auto machine = machine_t{};
    if(machine.is_active_state<idle>())
    {
        std::cout << "1\n";
    }

    machine.process_event(user_interface::start_request{});
    if(machine.is_active_state<starting>())
    {
        std::cout << "2\n";
    }

    machine.process_event(motor::start_event{});
    if(machine.is_active_state<running>())
    {
        std::cout << "3\n";
    }

    machine.process_event(user_interface::stop_request{});
    if(machine.is_active_state<stopping>())
    {
        std::cout << "4\n";
    }

    machine.process_event(motor::stop_event{});
    if(machine.is_active_state<idle>())
    {
        std::cout << "5\n";
    }
}
