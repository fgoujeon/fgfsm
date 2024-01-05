//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"
#include <string>

namespace
{
    struct context
    {
        std::string out;
    };

    namespace events
    {
        struct button_press
        {
            std::string data;
        };

        struct alert_button_press
        {
        };
    }

    namespace states
    {
        struct off_data
        {
            template<class Event>
            void on_event(const Event& /*event*/)
            {
            }

            void on_event(const events::button_press& event)
            {
                ctx.out += event.data + "2;";
            }

            context& ctx;
        };

        constexpr auto off = maki::state_conf{}
            .data<off_data>()
            .internal_action_de<maki::any>
            (
                [](off_data& dat, const auto& event)
                {
                    dat.on_event(event);
                }
            )
        ;

        struct on_data
        {
            template<class Event>
            void on_event(const Event& /*event*/)
            {
            }

            void on_event(const events::button_press& /*event*/)
            {
                ctx.out += "_";
            }

            void on_event(const events::alert_button_press& /*event*/)
            {
                ctx.out += "beep;";
            }

            context& ctx;
        };

        constexpr auto on = maki::state_conf{}
            .data<on_data>()
            .internal_action_de<maki::any>
            (
                [](on_data& dat, const auto& event)
                {
                    dat.on_event(event);
                }
            )
        ;
    }

    constexpr auto transition_table = maki::transition_table{}
        .add_c<states::off, events::button_press, states::on>
        .add_c<states::on,  events::button_press, states::off>
    ;

    constexpr auto machine_conf = maki::machine_conf{}
        .transition_tables(transition_table)
        .context<context>()
        .event_action_ce<events::button_press>
        (
            [](context& ctx, const events::button_press& event)
            {
                ctx.out += event.data + "1;";
            }
        )
    ;

    using machine_t = maki::machine<machine_conf>;
}

TEST_CASE("on_event_auto")
{
    auto machine = machine_t{};
    auto& ctx = machine.context();

    machine.start();

    ctx.out.clear();
    machine.process_event(events::button_press{"a"});
    REQUIRE(machine.active_state<states::on>());
    REQUIRE(ctx.out == "a1;");

    ctx.out.clear();
    machine.process_event(events::alert_button_press{});
    REQUIRE(machine.active_state<states::on>());
    REQUIRE(ctx.out == "beep;");
}
