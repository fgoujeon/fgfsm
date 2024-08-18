//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"

namespace submachine_ns
{
    struct machine_conf_holder;
    using machine_t = maki::machine<machine_conf_holder>;

    enum class led_color
    {
        off,
        red,
        green,
        blue
    };

    struct context
    {
        led_color current_led_color = led_color::off;
    };

    namespace events
    {
        struct power_button_press{};
        struct color_button_press{};
    }

    namespace states
    {
        EMPTY_STATE(off)

        struct emitting_red_data
        {
            context& ctx;
            machine_t& machine;
        };

        constexpr auto emitting_red = maki::state_conf{}
            .context_cm(maki::type<emitting_red_data>)
            .entry_action_c
            (
                maki::any,
                [](emitting_red_data& ctx)
                {
                    ctx.ctx.current_led_color = led_color::red;
                }
            )
        ;

        struct emitting_green_data
        {
            context& ctx;
            machine_t& machine;
        };

        constexpr auto emitting_green = maki::state_conf{}
            .context_cm(maki::type<emitting_green_data>)
            .entry_action_c
            (
                maki::any,
                [](emitting_green_data& ctx)
                {
                    ctx.ctx.current_led_color = led_color::green;
                }
            )
        ;

        struct emitting_blue_data
        {
            context& ctx;
            machine_t& machine;
        };

        constexpr auto emitting_blue = maki::state_conf{}
            .context_cm(maki::type<emitting_blue_data>)
            .entry_action_c
            (
                maki::any,
                [](emitting_blue_data& ctx)
                {
                    ctx.ctx.current_led_color = led_color::blue;
                }
            )
        ;

        constexpr auto on_transition_table = maki::transition_table{}
            (states::emitting_red,   maki::type<events::color_button_press>, states::emitting_green)
            (states::emitting_green, maki::type<events::color_button_press>, states::emitting_blue)
            (states::emitting_blue,  maki::type<events::color_button_press>, states::emitting_red)
        ;

        constexpr auto on = maki::submachine_conf{}
            .transition_tables(on_transition_table)
            .exit_action_c
            (
                maki::any,
                [](context& ctx)
                {
                    ctx.current_led_color = led_color::off;
                }
            )
        ;
    }

    constexpr auto transition_table = maki::transition_table{}
        (states::off, maki::type<events::power_button_press>, states::on)
        (states::on,  maki::type<events::power_button_press>, states::off)
    ;

    struct machine_conf_holder
    {
        static constexpr auto conf = maki::machine_conf{}
            .transition_tables(transition_table)
            .context_a(maki::type<context>)
        ;
    };
}

TEST_CASE("submachine")
{
    using namespace submachine_ns;

    auto machine = machine_t{};
    auto& ctx = machine.context();
    const auto on_state = machine.state<states::on>();

    machine.start();

    REQUIRE(machine.is<states::off>());
    REQUIRE(!on_state.running());
    REQUIRE(ctx.current_led_color == led_color::off);

    machine.process_event(events::power_button_press{});
    REQUIRE(machine.is<states::on>());
    REQUIRE(on_state.is<states::emitting_red>());
    REQUIRE(ctx.current_led_color == led_color::red);

    machine.process_event(events::color_button_press{});
    REQUIRE(machine.is<states::on>());
    REQUIRE(ctx.current_led_color == led_color::green);

    machine.process_event(events::color_button_press{});
    REQUIRE(machine.is<states::on>());
    REQUIRE(ctx.current_led_color == led_color::blue);

    machine.process_event(events::power_button_press{});
    REQUIRE(machine.is<states::off>());
    REQUIRE(ctx.current_led_color == led_color::off);

    machine.process_event(events::power_button_press{});
    REQUIRE(machine.is<states::on>());
    REQUIRE(ctx.current_led_color == led_color::red);
}
