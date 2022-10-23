//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common/catch.hpp"
#include <string>

namespace
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
        std::string out;
    };

    namespace states
    {
        struct on{};
        struct off{};
    }

    template<class State>
    std::string get_state_name()
    {
        if constexpr(std::is_same_v<State, states::on>)
        {
            return "on";
        }
        else if constexpr(std::is_same_v<State, states::off>)
        {
            return "off";
        }
        else if constexpr(std::is_same_v<State, awesm::null_state>)
        {
            return "null";
        }
    }

    using sm_transition_table = awesm::transition_table
    <
        awesm::row<states::off, events::button_press, states::on>,
        awesm::row<states::on,  events::button_press, states::off>
    >;

    struct sm_def
    {
        using transition_tables = awesm::transition_table_list<sm_transition_table>;

        template<int RegionIndex, class SourceState, class Event, class TargetState>
        void before_state_transition(const Event& event)
        {
            static_assert(RegionIndex == 0);

            ctx.out += get_state_name<SourceState>() + "->" + get_state_name<TargetState>() + "...;";
            ctx.out += std::to_string(event.pressure) + ";";
        }

        context& ctx;
    };

    struct sm_after_state_transition;

    using sm_t = awesm::sm
    <
        sm_def,
        awesm::sm_options::before_state_transition,
        awesm::sm_options::after_state_transition<sm_after_state_transition>
    >;

    struct sm_after_state_transition
    {
        template<int RegionIndex, class SourceState, class Event, class TargetState>
        void after_state_transition(const Event& event)
        {
            static_assert(RegionIndex == 0);

            ctx.out += std::to_string(event.pressure) + ";";
            ctx.out += get_state_name<SourceState>() + "->" + get_state_name<TargetState>() + ";";
        }

        sm_t& sm;
        context& ctx;
    };
}

TEST_CASE("state_transition_hook_set")
{
    auto ctx = context{};
    auto sm = sm_t{ctx};

    sm.start(events::button_press{0});
    REQUIRE(sm.is_active_state<states::off>());
    REQUIRE(ctx.out == "null->off...;0;0;null->off;");

    ctx.out.clear();
    sm.process_event(events::button_press{1});
    REQUIRE(sm.is_active_state<states::on>());
    REQUIRE(ctx.out == "off->on...;1;1;off->on;");

    ctx.out.clear();
    sm.process_event(events::button_press{2});
    REQUIRE(sm.is_active_state<states::off>());
    REQUIRE(ctx.out == "on->off...;2;2;on->off;");
}
