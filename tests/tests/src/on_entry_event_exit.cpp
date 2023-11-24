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
        std::string hello;
        std::string dog;
        std::string goodbye;
    };

    namespace events
    {
        struct say_dog{};
        struct next_language_request{};
    }

    namespace states
    {
        EMPTY_STATE(idle);

        struct english
        {
            static constexpr auto conf = maki::default_state_conf
                .entry_action_c<maki::any>
                (
                    [](context& ctx)
                    {
                        ctx.hello = "hello";
                    }
                )
                .enable_on_event_for<events::say_dog>()
                .exit_action_c<maki::any>
                (
                    [](context& ctx)
                    {
                        ctx.goodbye = "goodbye";
                    }
                )
            ;

            void on_event(const events::say_dog&)
            {
                ctx.dog = "dog";
            }

            context& ctx;
        };

        struct french
        {
            static constexpr auto conf = maki::default_state_conf
                .entry_action_m<maki::any>
                (
                    [](auto& mach)
                    {
                        mach.context().hello = "bonjour";
                    }
                )
                .enable_on_event_for<events::say_dog>()
                .exit_action_m<maki::any>
                (
                    [](auto& mach)
                    {
                        mach.context().goodbye = "au revoir";
                    }
                )
            ;

            void on_event(const events::say_dog&)
            {
                ctx.dog = "chien";
            }

            context& ctx;
        };
    }

    constexpr auto transition_table = maki::empty_transition_table
        .add_c<states::idle,    events::next_language_request, states::english>
        .add_c<states::english, events::next_language_request, states::french>
        .add_c<states::french,  events::next_language_request, states::idle>
    ;

    struct machine_def
    {
        static constexpr auto conf = maki::default_machine_conf
            .set_transition_tables(transition_table)
            .set_context<context>()
        ;
    };

    using machine_t = maki::machine<machine_def>;
}

TEST_CASE("entry_action_event_exit")
{
    auto machine = machine_t{};
    auto& ctx = machine.context();

    machine.start();

    REQUIRE(machine.is_active_state<states::idle>());
    REQUIRE(ctx.hello == "");
    REQUIRE(ctx.dog == "");
    REQUIRE(ctx.goodbye == "");

    machine.process_event(events::next_language_request{});
    REQUIRE(machine.is_active_state<states::english>());
    REQUIRE(ctx.hello == "hello");
    REQUIRE(ctx.dog == "");
    REQUIRE(ctx.goodbye == "");

    machine.process_event(events::say_dog{});
    REQUIRE(machine.is_active_state<states::english>());
    REQUIRE(ctx.hello == "hello");
    REQUIRE(ctx.dog == "dog");
    REQUIRE(ctx.goodbye == "");

    machine.process_event(events::next_language_request{});
    REQUIRE(machine.is_active_state<states::french>());
    REQUIRE(ctx.hello == "bonjour");
    REQUIRE(ctx.dog == "dog");
    REQUIRE(ctx.goodbye == "goodbye");

    machine.process_event(events::say_dog{});
    REQUIRE(machine.is_active_state<states::french>());
    REQUIRE(ctx.hello == "bonjour");
    REQUIRE(ctx.dog == "chien");
    REQUIRE(ctx.goodbye == "goodbye");

    machine.process_event(events::next_language_request{});
    REQUIRE(machine.is_active_state<states::idle>());
    REQUIRE(ctx.hello == "bonjour");
    REQUIRE(ctx.dog == "chien");
    REQUIRE(ctx.goodbye == "au revoir");
}
