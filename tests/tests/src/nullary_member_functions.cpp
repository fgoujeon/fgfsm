//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"
#include <string>

namespace nullary_member_functions_ns
{
    struct context
    {
        std::string out;
    };

    namespace events
    {
        struct e1
        {
        };

        struct e2
        {
        };
    }

    namespace states
    {
        EMPTY_STATE(off)

        constexpr auto on = maki::state_conf{}
            .entry_action_c
            (
                maki::type<events::e1>,
                [](context& ctx)
                {
                    ctx.out += "on_entry(e1);";
                }
            )
            .entry_action_c
            (
                maki::any,
                [](context& ctx)
                {
                    ctx.out += "on_entry();";
                }
            )
            .exit_action_c
            (
                maki::type<events::e1>,
                [](context& ctx)
                {
                    ctx.out += "on_exit(e1);";
                }
            )
            .exit_action_c
            (
                maki::any,
                [](context& ctx)
                {
                    ctx.out += "on_exit();";
                }
            )
        ;
    }

    constexpr auto action = [](context& ctx, auto& /*machine*/, const auto& event)
    {
        using event_type = std::decay_t<decltype(event)>;
        if constexpr(std::is_same_v<event_type, events::e1>)
        {
            ctx.out += "execute(e1);";
        }
        else
        {
            ctx.out += "execute();";
        }
    };

    constexpr auto guard = [](context& ctx, auto& /*machine*/, const auto& event)
    {
        using event_type = std::decay_t<decltype(event)>;
        if constexpr(std::is_same_v<event_type, events::e1>)
        {
            ctx.out += "check(e1);";
        }
        else
        {
            ctx.out += "check();";
        }
        return true;
    };

    constexpr auto make_transition_table()
    {
        return maki::transition_table{}
        (states::off, maki::type<events::e1>, states::on,  action, guard)
        (states::off, maki::type<events::e2>, states::on,  action, guard)
        (states::on,  maki::type<events::e1>, states::off, action, guard)
        (states::on,  maki::type<events::e2>, states::off, action, guard)
        ;
    }

    constexpr auto machine_conf = maki::machine_conf{}
        .transition_tables(make_transition_table)
        .context_a(maki::type<context>)
    ;

    using machine_t = maki::make_machine<machine_conf>;
}

TEST_CASE("nullary_member_functions")
{
    using namespace nullary_member_functions_ns;

    auto machine = machine_t{};
    auto& ctx = machine.context();

    machine.start();

    ctx.out.clear();
    machine.process_event(events::e1{});
    REQUIRE(ctx.out == "check(e1);execute(e1);on_entry(e1);");

    ctx.out.clear();
    machine.process_event(events::e1{});
    REQUIRE(ctx.out == "check(e1);on_exit(e1);execute(e1);");

    ctx.out.clear();
    machine.process_event(events::e2{});
    REQUIRE(ctx.out == "check();execute();on_entry();");

    ctx.out.clear();
    machine.process_event(events::e2{});
    REQUIRE(ctx.out == "check();on_exit();execute();");
}
