//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include <functional>
#include <iostream>

/*
This class monitors the button. It sends an event of type push_event whenever
the user pushes the button.
*/
class button
{
public:
    struct push_event
    {
        //Push duration, in milliseconds
        int duration_ms = 0;
    };

    using event_handler = std::function<void(const push_event&)>;

    button(const event_handler& /*eh*/)
    {
        //Implementation detail...
    }

private:
    //Implementation detail...
};

//This class drives the RGB LED.
class rgb_led
{
public:
    enum class color
    {
        off,
        white,
        red,
        green,
        blue
    };

    color get_color() const
    {
        return color_;
    }

    void set_color(const color c)
    {
        color_ = c;
        //Implementation detail...
    }

private:
    color color_ = color::off;
    //Implementation detail...
};

/*
An instance of this class is instantiated by the constructor of the state
machine. This instance can be accessed by every element (states, actions,
guards, ...) of the state machine.
*/
struct context
{
    rgb_led led;
};

/*
States are represented by constexpr objects.
*/
namespace states
{
    constexpr auto off = maki::state_conf{}
        /*
        Entry action invoked whenever the state machine enters the `off` state
        with a `button::push_event`.
        */
        .entry_action_e(maki::type<button::push_event>, [](const button::push_event& event)
        {
            std::cout << "Turned off after a ";
            std::cout << event.duration_ms << " millisecond push\n";
        })

        /*
        Entry action invoked whenever the state machine enters the `off` state
        with a state machine start event.
        */
        .entry_action_v(maki::type<maki::events::start>, []
        {
            std::cout << "Started state machine\n";
        })

        /*
        Internal action invoked whenever a `button::push_event` occurs while
        the `off` state is active.
        */
        .internal_action_e(maki::type<button::push_event>, [](const button::push_event& event)
        {
            std::cout << "Received a ";
            std::cout << event.duration_ms;
            std::cout << " millisecond push in off state\n";
        })

        /*
        Exit action invoked whenever the state machine exits the `off` state,
        whatever the type of the event that caused the state transition.
        */
        .exit_action_v(maki::any, []
        {
            std::cout << "Turned on\n";
        })
    ;

    /*
    States can have their own private data.
    */
    struct emitting_white_data
    {
        int counter = 0;
    };
    constexpr auto emitting_white = maki::state_conf{}
        .context_v(maki::type<emitting_white_data>)
        .entry_action_c(maki::any, [](emitting_white_data& data)
        {
            ++data.counter;
        })
    ;

    /*
    These are minimal valid state classes.
    */
    constexpr auto emitting_red = maki::state_conf{};
    constexpr auto emitting_green = maki::state_conf{};
    constexpr auto emitting_blue = maki::state_conf{};
}

/*
An action is a callable invoked whenever a specific state transition occurs.
*/
namespace actions
{
    /*
    One of the following expressions must be valid:
        action(context, machine, event);
        action(context, event);
        action(context);
    */
    void turn_light_off(context& ctx)
    {
        ctx.led.set_color(rgb_led::color::off);
    }

    //We can of course factorize with a template.
    template<auto Color>
    void turn_light_tpl(context& ctx)
    {
        ctx.led.set_color(Color);
    }
    constexpr auto turn_light_white = turn_light_tpl<rgb_led::color::white>;
    constexpr auto turn_light_red   = turn_light_tpl<rgb_led::color::red>;
    constexpr auto turn_light_green = turn_light_tpl<rgb_led::color::green>;
    constexpr auto turn_light_blue  = turn_light_tpl<rgb_led::color::blue>;
}

/*
A guard is a callable invoked to check that a state transition can occur.
*/
namespace guards
{
    /*
    One of the following expressions must be valid:
        guard(context, machine, event);
        guard(context, event);
        guard(context);
    */
    bool is_long_push(context& /*ctx*/, const button::push_event& event)
    {
        return event.duration_ms > 1000;
    }

    //We can use maki::guard and boolean operators to compose guards.
    constexpr auto is_short_push = !maki::guard{is_long_push};
}

/*
This is the transition table. This is where we define the actions that the state
machine must execute depending on the active state and the event it receives.

Basically, whenever `maki::machine::process_event()` is called, Maki iterates
over the transitions of this table until it finds a match, i.e. when:
- `source_state` is the currently active state;
- `event` is the type of the processed event;
- and `guard` returns true.

When a match is found, Maki:
- exits `source_state`;
- marks `target_state` as the new active state;
- invokes `action`;
- enters `target_state`.

The initial active state of the state machine is the first state encountered in
the transition table (`off`, is our case).
*/
constexpr auto make_transition_table()
{
    using namespace states;
    using namespace actions;
    using namespace guards;
    constexpr const auto& button_push = maki::type<button::push_event>;

    return maki::transition_table{}
        //source_state,      event,       target_state,   action,           guard
        (off,                button_push, emitting_white, turn_light_white)
        (emitting_white,     button_push, emitting_red,   turn_light_red,   is_short_push)
        (emitting_red,       button_push, emitting_green, turn_light_green, is_short_push)
        (emitting_green,     button_push, emitting_blue,  turn_light_blue,  is_short_push)
        (emitting_blue,      button_push, emitting_white, turn_light_white, is_short_push)
        (maki::any_but(off), button_push, off,            turn_light_off,   is_long_push)
    ;
}

/*
We have to define this variable to configure our state machine.
Here, we just specify the transition table and the context type, but we can
configure many other aspects of the state machine.
*/
constexpr auto machine_conf = maki::machine_conf{}
    .transition_tables(make_transition_table)
    .context_a(maki::type<context>)
;

/*
We finally have our configured state machine.
*/
using machine_t = maki::make_machine<machine_conf>;

int main()
{
    /*
    When we instantiate the state machine, we also instantiate:
    - a context;
    - the data types of the states mentioned in the transition table.

    Note that the state data types are instantiated once and for all: no
    construction or destruction happens during state transitions.
    */
    auto machine = machine_t{};
    auto& ctx = machine.context();

#if TESTING
    auto simulate_push = [&](const int duration_ms)
    {
        std::cout << "Button push (" << duration_ms << " ms)\n";
        machine.process_event(button::push_event{duration_ms});
    };

    auto check = [](const bool b)
    {
        if(!b)
        {
            std::cerr << "NOK\n";
            std::cerr << "Test failed\n";
            exit(1);
        }

        std::cout << "OK\n";
    };

    check(machine.active_state<states::off>());
    check(ctx.led.get_color() == rgb_led::color::off);

    simulate_push(200);
    check(machine.active_state<states::emitting_white>());
    check(ctx.led.get_color() == rgb_led::color::white);

    simulate_push(200);
    check(machine.active_state<states::emitting_red>());
    check(ctx.led.get_color() == rgb_led::color::red);

    simulate_push(200);
    check(machine.active_state<states::emitting_green>());
    check(ctx.led.get_color() == rgb_led::color::green);

    simulate_push(200);
    check(machine.active_state<states::emitting_blue>());
    check(ctx.led.get_color() == rgb_led::color::blue);

    simulate_push(200);
    check(machine.active_state<states::emitting_white>());
    check(ctx.led.get_color() == rgb_led::color::white);

    simulate_push(1500);
    check(machine.active_state<states::off>());
    check(ctx.led.get_color() == rgb_led::color::off);

    std::cout << "Test succeeded\n";

    return 0;
#else
    /*
    A real-life program would do something like this.
    */

    auto btn = button
    {
        [&](const auto& event)
        {
            machine.process_event(event);
        }
    };

    while(true){}
#endif
}
