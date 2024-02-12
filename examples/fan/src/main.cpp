//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

//! [all]
#include <maki.hpp>
#include <functional>
#include <iostream>

struct context{};

//! [events-and-datatypes]
//Data types
enum class speed
{
    low,
    med,
    high
};

//Events
struct memory_read
{
    speed spd = speed::low;
};
struct minus_button_press{};
struct plus_button_press{};
//! [events-and-datatypes]

//States
constexpr auto reading_memory = maki::state_conf{};
constexpr auto spinning_low = maki::state_conf{}
    .entry_action_v(maki::any, []
    {
        std::cout << "Speed is low\n";

        //Set fan speed and save speed in memory
        //[Implementation detail...]
    })
;
constexpr auto spinning_med = maki::state_conf{}
    .entry_action_v(maki::any, []
    {
        std::cout << "Speed is med\n";

        //Set fan speed and save speed in memory
        //[Implementation detail...]
    })
;
constexpr auto spinning_high = maki::state_conf{}
    .entry_action_v(maki::any, []
    {
        std::cout << "Speed is high\n";

        //Set fan speed and save speed in memory
        //[Implementation detail...]
    })
;

//Guards
//! [guards]
bool is_speed_low(context& /*ctx*/, const memory_read& event)
{
    return event.spd == speed::low;
}
bool is_speed_med(context& /*ctx*/, const memory_read& event)
{
    return event.spd == speed::med;
}
bool is_speed_high(context& /*ctx*/, const memory_read& event)
{
    return event.spd == speed::high;
}
//! [guards]

//Transition table
//! [transition-table]
constexpr auto make_transition_table()
{
    return maki::transition_table{}
        //source state,  event,                          target state,  action,     guard
        (reading_memory, maki::type<memory_read>,        spinning_low,  maki::null, is_speed_low)
        (reading_memory, maki::type<memory_read>,        spinning_med,  maki::null, is_speed_med)
        (reading_memory, maki::type<memory_read>,        spinning_high, maki::null, is_speed_high)
        (spinning_low,   maki::type<plus_button_press>,  spinning_med,  maki::null)
        (spinning_med,   maki::type<plus_button_press>,  spinning_high, maki::null)
        (spinning_med,   maki::type<minus_button_press>, spinning_low,  maki::null)
        (spinning_high,  maki::type<minus_button_press>, spinning_med,  maki::null)
    ;
}
//! [transition-table]

//State machine configuration
constexpr auto machine_conf = maki::machine_conf{}
    .transition_tables(make_transition_table)
    .context_a(maki::type<context>)
;

//State machine
using machine_t = maki::make_machine<machine_conf>;

int main()
{
    auto machine = machine_t{};

    //Simulate a memory read that returns a "med" speed.
    //This eventually sets the fan speed to "med".
    machine.process_event(memory_read{speed::med});

    //Simulate button presses
    {
        //Set fan speed to "high"
        machine.process_event(plus_button_press{});

        //Doesn't do anything, as the highest possible speed is already reached
        machine.process_event(plus_button_press{});
        machine.process_event(plus_button_press{});

        //Set fan speed to "med"
        machine.process_event(minus_button_press{});

        //Set fan speed to "low"
        machine.process_event(minus_button_press{});

        //Doesn't do anything, as the lowest possible speed is already reached
        machine.process_event(minus_button_press{});
    }

    return 0;
}
//! [all]
