//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common/catch.hpp"

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
        };

        struct exception_request
        {
        };
    }

    namespace states
    {
        struct off0{};
        struct off1{};
        struct on0{};
        struct on1
        {
            void on_event(const events::exception_request&)
            {
                throw std::runtime_error{"exception"};
            }
        };
    }

    struct sm_def
    {
        using transition_tables = awesm::transition_table_list
        <
            awesm::transition_table
            <
                awesm::row<states::off0, events::button_press, states::on0>
            >,
            awesm::transition_table
            <
                awesm::row<states::off1, events::button_press, states::on1>
            >
        >;

        template<int RegionIndex, class SourceState, class Event, class TargetState>
        void before_state_transition(const Event& /*event*/)
        {
            ctx.out += "before_state_transition[" + std::to_string(RegionIndex) + "];";
        }

        context& ctx;
    };

    struct sm_on_exception;
    struct sm_after_state_transition;

    using sm_t = awesm::sm
    <
        sm_def,
        awesm::sm_options::on_exception<sm_on_exception>,
        awesm::sm_options::before_state_transition,
        awesm::sm_options::after_state_transition<sm_after_state_transition>
    >;

    struct sm_on_exception
    {
        void on_exception(const std::exception_ptr& eptr)
        {
            try
            {
                std::rethrow_exception(eptr);
            }
            catch(const std::exception& e)
            {
                ctx.out += std::string{"on_exception:"} + e.what() + ";";
            }
        }

        context& ctx;
    };

    struct sm_after_state_transition
    {
        template<int RegionIndex, class SourceState, class Event, class TargetState>
        void after_state_transition(const Event& /*event*/)
        {
            ctx.out += "after_state_transition[" + std::to_string(RegionIndex) + "];";
        }

        sm_t& sm;
        context& ctx;
    };
}

TEST_CASE("orthogonal_regions")
{
    auto ctx = context{};
    auto sm = sm_t{ctx};

    sm.start();
    REQUIRE(sm.is_active_state<states::off0, 0>());
    REQUIRE(sm.is_active_state<states::off1, 1>());
    REQUIRE(ctx.out == "before_state_transition[0];after_state_transition[0];before_state_transition[1];after_state_transition[1];");

    ctx.out.clear();
    sm.process_event(events::button_press{});
    REQUIRE(sm.is_active_state<states::on0, 0>());
    REQUIRE(sm.is_active_state<states::on1, 1>());
    REQUIRE(ctx.out == "before_state_transition[0];after_state_transition[0];before_state_transition[1];after_state_transition[1];");

    ctx.out.clear();
    sm.process_event(events::exception_request{});
    REQUIRE(ctx.out == "on_exception:exception;");
}
