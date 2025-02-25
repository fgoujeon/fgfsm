//Copyright Florian Goujeon 2021 - 2025.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"
#include <string>

namespace double_start_stop_ns
{
    namespace events
    {
        struct button_press
        {
            int pressure = 0;
        };
    }

    struct context
    {
        std::string& out;
    };

    namespace states
    {
        EMPTY_STATE(on)
        EMPTY_STATE(off)
    }

    constexpr auto transition_table = maki::transition_table{}
        (maki::init,  states::off)
        (states::off, states::on,  maki::event<events::button_press>)
        (states::on,  states::off, maki::event<events::button_press>)
    ;

    constexpr auto machine_conf = maki::machine_conf{}
        .transition_tables(transition_table)
        .context_a<context>()
        .pre_external_transition_hook_crset
        (
            [](context& ctx, const auto& region, const auto& source_state, const auto& /*event*/, const auto& target_state)
            {
                //REQUIRE(path_constant.value == maki::path<maki::path_element<machine_def, 0>>{});

                ctx.out += "Transition in main_sm/";
                ctx.out += region.path().to_string();
                ctx.out += ": ";
                ctx.out += source_state.pretty_name();
                ctx.out += " -> ";
                ctx.out += target_state.pretty_name();
                ctx.out += "...;";
            }
        )
        .post_external_transition_hook_crset
        (
            [](context& ctx, const auto& region, const auto& source_state, const auto& /*event*/, const auto& target_state)
            {
                //REQUIRE(path_constant.value == maki::path<maki::path_element<machine_def, 0>>{});

                ctx.out += "Transition in main_sm/";
                ctx.out += region.path().to_string();
                ctx.out += ": ";
                ctx.out += source_state.pretty_name();
                ctx.out += " -> ";
                ctx.out += target_state.pretty_name();
                ctx.out += ";";
            }
        )
    ;

    using machine_t = maki::machine<machine_conf>;
}

TEST_CASE("double_start_stop")
{
    using namespace double_start_stop_ns;

    auto out = std::string{};
    auto machine = machine_t{out};

    machine.start();
    machine.start();

    REQUIRE(machine.is<states::off>());
    REQUIRE
    (
        out ==
        "Transition in main_sm/0: init -> off...;"
        "Transition in main_sm/0: init -> off;"
    );

    out.clear();
    machine.stop();
    REQUIRE(!machine.running());
    REQUIRE
    (
        out ==
        "Transition in main_sm/0: off -> fin...;"
        "Transition in main_sm/0: off -> fin;"
    );

    out.clear();
    machine.stop();
    REQUIRE(!machine.running());
    REQUIRE(out == "");
}
