//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"

namespace
{
    struct context
    {
        bool exception_thrown = false;
        std::string out;
    };

    namespace events
    {
        struct button_press{};
    }

    namespace states
    {
        constexpr auto off = maki::state_conf_c
            .entry_action_c<maki::any>
            (
                [](context& ctx)
                {
                    ctx.out += "off::on_entry;";
                }
            )
            .exit_action_c<maki::any>
            (
                [](context& ctx)
                {
                    ctx.out += "off::on_exit;";

                    if(!ctx.exception_thrown)
                    {
                        ctx.exception_thrown = true;
                        throw std::runtime_error{"error"};
                    }
                }
            )
        ;

        constexpr auto on = maki::state_conf_c
            .entry_action_c<maki::any>
            (
                [](context& ctx)
                {
                    ctx.out += "on::on_entry;";
                }
            )
            .exit_action_c<maki::any>
            (
                [](context& ctx)
                {
                    ctx.out += "on::on_exit;";
                }
            )
        ;
    }

    namespace actions
    {
        void unreachable(context& ctx)
        {
            ctx.out += "unreachable;";
        }
    }

    constexpr auto transition_table = maki::transition_table_c
        .add_c<states::off, events::button_press, states::on>
        .add_c<states::off, events::button_press, states::on,  actions::unreachable>
        .add_c<states::on,  events::button_press, states::off, actions::unreachable>
    ;

    struct machine_def
    {
        static constexpr auto conf = maki::machine_conf_c
            .transition_tables(transition_table)
            .context<context>()
        ;
    };

    using machine_t = maki::machine<machine_def>;
}

TEST_CASE("basic_exception")
{
    auto machine = machine_t{};

    REQUIRE(machine.is_active_state<states::off>());
    REQUIRE(machine.context().out == "off::on_entry;");

    machine.context().out.clear();
    machine.process_event(events::button_press{});
    REQUIRE(machine.is_active_state<states::off>());
    REQUIRE(machine.context().out == "off::on_exit;");
}
