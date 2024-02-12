//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"

namespace
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
        constexpr auto off = maki::state_conf{}
            .entry_action_c
            (
                maki::any,
                [](context& ctx)
                {
                    ctx.current_led_color = led_color::off;
                }
            )
        ;

        constexpr auto emitting_red = maki::state_conf{}
            .entry_action_c
            (
                maki::any,
                [](context& ctx)
                {
                    ctx.current_led_color = led_color::red;
                }
            )
        ;

        constexpr auto emitting_green = maki::state_conf{}
            .entry_action_c
            (
                maki::any,
                [](context& ctx)
                {
                    ctx.current_led_color = led_color::green;
                }
            )
        ;

        constexpr auto emitting_blue = maki::state_conf{}
            .entry_action_c
            (
                maki::any,
                [](context& ctx)
                {
                    ctx.current_led_color = led_color::blue;
                }
            )
        ;

        constexpr auto make_on_transition_table()
        {
            return maki::transition_table{}
                (states::emitting_red,   maki::type<events::color_button_press>, states::emitting_green)
                (states::emitting_green, maki::type<events::color_button_press>, states::emitting_blue)
                (states::emitting_blue,  maki::type<events::color_button_press>, states::emitting_red)
            ;
        }

        constexpr auto on = maki::submachine_conf{}
            .transition_tables(make_on_transition_table)
        ;
    }

    constexpr auto make_transition_table()
    {
        return maki::transition_table{}
        (states::on, maki::type<events::power_button_press>, states::off)
        ;
    }

    constexpr auto machine_conf = maki::machine_conf{}
        .transition_tables(make_transition_table)
        .context_a(maki::type<context>)
    ;

    struct machine_conf_holder: maki::conf_holder<machine_conf>{};
}

TEST_CASE("initial_submachine")
{
    auto machine = machine_t{};
    auto& ctx = machine.context();

    static constexpr auto on_path = maki::path{0} / states::on / 0;

    machine.start();
    REQUIRE(machine.active_state<states::on>());
    REQUIRE(machine.active_state<on_path, states::emitting_red>());
    REQUIRE(ctx.current_led_color == led_color::red);

    machine.process_event(events::color_button_press{});
    REQUIRE(machine.active_state<states::on>());
    REQUIRE(ctx.current_led_color == led_color::green);

    machine.process_event(events::color_button_press{});
    REQUIRE(machine.active_state<states::on>());
    REQUIRE(ctx.current_led_color == led_color::blue);

    machine.process_event(events::power_button_press{});
    REQUIRE(machine.active_state<states::off>());
    REQUIRE(ctx.current_led_color == led_color::off);
}
