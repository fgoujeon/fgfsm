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
        int always_zero = 0;
        std::string out;
    };

    namespace states
    {
        constexpr auto off = maki::state_conf{}
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
                }
            )
        ;

        constexpr auto on = maki::state_conf{}
            .entry_action_c<maki::any>
            (
                [](context& ctx)
                {
                    ctx.out += "on::on_entry;";

                    if(ctx.always_zero == 0) //We need this to avoid "unreachable code" warnings
                    {
                        throw std::runtime_error{"test;"};
                    }
                }
            )
            .internal_action_ce<maki::events::exception>
            (
                [](context& ctx, const maki::events::exception& event)
                {
                    try
                    {
                        std::rethrow_exception(event.eptr);
                    }
                    catch(const std::exception& e)
                    {
                        ctx.out += "default;";
                        ctx.out += e.what();
                    }
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

    namespace events
    {
        struct button_press{};
    }

    constexpr auto transition_table = maki::transition_table{}
        .add_c<states::off, events::button_press, states::on>
        .add_c<states::on,  events::button_press, states::off>
    ;

    struct default_sm_def
    {
        static constexpr auto conf = maki::machine_conf{}
            .transition_tables(transition_table)
            .context<context>()
        ;
    };

    using default_sm_t = maki::machine<default_sm_def>;

    struct custom_sm_def
    {
        static constexpr auto conf = maki::machine_conf{}
            .transition_tables(transition_table)
            .context<context>()
            .exception_action_me
            (
                [](auto& mach, const std::exception_ptr& eptr)
                {
                    try
                    {
                        std::rethrow_exception(eptr);
                    }
                    catch(const std::exception& e)
                    {
                        mach.context().out += "custom;";
                        mach.context().out += e.what();
                    }
                }
            )
        ;
    };

    using custom_sm_t = maki::machine<custom_sm_def>;
}

TEST_CASE("on_exception")
{
    {
        auto machine = default_sm_t{};
        auto& ctx = machine.context();

        machine.start();
        ctx.out.clear();

        machine.process_event(events::button_press{});
        REQUIRE(machine.active_state<states::on>());
        REQUIRE(ctx.out == "off::on_exit;on::on_entry;default;test;");
    }

    {
        auto machine = custom_sm_t{};
        auto& ctx = machine.context();

        machine.start();
        ctx.out.clear();

        machine.process_event(events::button_press{});
        REQUIRE(machine.active_state<states::on>());
        REQUIRE(ctx.out == "off::on_exit;on::on_entry;custom;test;");
    }
}
