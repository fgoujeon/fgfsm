//Copyright Florian Goujeon 2021 - 2024.
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
        bool can_access_state0_0 = false;
        bool can_access_state0_1 = false;

        bool can_access_state1_0 = false;
        bool can_access_state1_1 = false;

        bool can_access_state2_0 = false;
        bool can_access_state2_1 = false;

        bool cant_access_state3 = true;
    };

    namespace states
    {
        EMPTY_STATE(idle)
        EMPTY_STATE(state0)
        EMPTY_STATE(state1)
        EMPTY_STATE(state2)
        EMPTY_STATE(state3)
    }

    namespace events
    {
        struct start{};
        struct stop{};
    }

    namespace guards
    {

#define GUARD(NAME) \
    bool NAME(context& ctx) \
    { \
        return ctx.NAME; \
    }

//Test with another signature
#define GUARD_2(NAME) \
    inline constexpr auto NAME = [](context& ctx, auto& /*machine*/, const auto& /*event*/) \
    { \
        return ctx.NAME; \
    };

        GUARD(can_access_state0_0)
        GUARD_2(can_access_state0_1)
        GUARD(can_access_state1_0)
        GUARD_2(can_access_state1_1)
        GUARD(can_access_state2_0)
        GUARD_2(can_access_state2_1)
        GUARD(cant_access_state3)

#undef GUARD

        constexpr auto can_access_state0 = maki::guard{can_access_state0_0} && maki::guard{can_access_state0_1};
        constexpr auto can_access_state1 = maki::guard{can_access_state1_0} || maki::guard{can_access_state1_1};
        constexpr auto can_access_state2 = maki::guard{can_access_state2_0} != maki::guard{can_access_state2_1};
    }

    constexpr auto make_transition_table()
    {
        return maki::transition_table{}
        (states::idle, maki::type<events::start>, states::state0, maki::null, guards::can_access_state0)
        (states::idle, maki::type<events::start>, states::state1, maki::null, guards::can_access_state1)
        (states::idle, maki::type<events::start>, states::state2, maki::null, guards::can_access_state2)
        (states::idle, maki::type<events::start>, states::state3, maki::null, !maki::guard{guards::cant_access_state3})

        (states::state0, maki::type<events::stop>, states::idle)
        (states::state1, maki::type<events::stop>, states::idle)
        (states::state2, maki::type<events::stop>, states::idle)
        (states::state3, maki::type<events::stop>, states::idle)
        ;
    }

    constexpr auto machine_conf = maki::machine_conf{}
        .transition_tables(make_transition_table)
        .context_a(maki::type<context>)
    ;

    using machine_t = maki::make_machine<machine_conf>;
}

TEST_CASE("guard operators")
{
    auto machine = machine_t{};
    auto& ctx = machine.context();

    machine.start();

    SECTION("and")
    {
       machine.process_event(events::stop{});
       ctx.can_access_state0_0 = false;
       ctx.can_access_state0_1 = false;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::idle>());

       machine.process_event(events::stop{});
       ctx.can_access_state0_0 = false;
       ctx.can_access_state0_1 = true;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::idle>());

       machine.process_event(events::stop{});
       ctx.can_access_state0_0 = true;
       ctx.can_access_state0_1 = false;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::idle>());

       machine.process_event(events::stop{});
       ctx.can_access_state0_0 = true;
       ctx.can_access_state0_1 = true;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::state0>());
    }

    SECTION("or")
    {
       machine.process_event(events::stop{});
       ctx.can_access_state1_0 = false;
       ctx.can_access_state1_1 = false;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::idle>());

       machine.process_event(events::stop{});
       ctx.can_access_state1_0 = false;
       ctx.can_access_state1_1 = true;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::state1>());

       machine.process_event(events::stop{});
       ctx.can_access_state1_0 = true;
       ctx.can_access_state1_1 = false;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::state1>());

       machine.process_event(events::stop{});
       ctx.can_access_state1_0 = true;
       ctx.can_access_state1_1 = true;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::state1>());
    }

    SECTION("xor")
    {
       machine.process_event(events::stop{});
       ctx.can_access_state2_0 = false;
       ctx.can_access_state2_1 = false;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::idle>());

       machine.process_event(events::stop{});
       ctx.can_access_state2_0 = false;
       ctx.can_access_state2_1 = true;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::state2>());

       machine.process_event(events::stop{});
       ctx.can_access_state2_0 = true;
       ctx.can_access_state2_1 = false;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::state2>());

       machine.process_event(events::stop{});
       ctx.can_access_state2_0 = true;
       ctx.can_access_state2_1 = true;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::idle>());
    }

    SECTION("not")
    {
       machine.process_event(events::stop{});
       ctx.cant_access_state3 = true;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::idle>());

       machine.process_event(events::stop{});
       ctx.cant_access_state3 = false;
       machine.process_event(events::start{});
       REQUIRE(machine.active_state<states::state3>());
    }
}
