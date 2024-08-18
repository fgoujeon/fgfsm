//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"

namespace state_data_submachine_ns
{
    struct context
    {
    };

    namespace events
    {
        struct button_press{};

        struct accumulate_request
        {
            int n = 0;
        };
    }

    namespace states
    {
        EMPTY_STATE(off)
        EMPTY_STATE(on0)
        EMPTY_STATE(on1)

        constexpr auto on_transition_table = maki::transition_table{}
            (states::on0, maki::null, states::on1)
        ;

        struct on_data
        {
            int counter = 0;
        };

        void on_accumulate(on_data& self, const int n)
        {
            self.counter += n;
        }

        constexpr auto on = maki::submachine_conf{}
            .context_v(maki::type<on_data>)
            .transition_tables(on_transition_table)
            .internal_action_ce
            (
                maki::type<events::accumulate_request>,
                [](on_data& self, const events::accumulate_request& event)
                {
                    on_accumulate(self, event.n);
                }
            )
        ;
    }

    constexpr auto transition_table = maki::transition_table{}
        (states::off, maki::type<events::button_press>, states::on)
        (states::on,  maki::type<events::button_press>, states::off)
    ;

    constexpr auto machine_conf = maki::machine_conf{}
        .transition_tables(transition_table)
        .context_a(maki::type<context>)
    ;

    using machine_t = maki::make_machine<machine_conf>;
}

TEST_CASE("state_data_submachine")
{
    using namespace state_data_submachine_ns;

    auto machine = machine_t{};
    const auto on_state = machine.state<states::on>();
    auto& counter = on_state.context().counter;

    machine.process_event(events::button_press{});
    REQUIRE(counter == 0);

    machine.process_event(events::accumulate_request{1});
    REQUIRE(counter == 1);

    machine.process_event(events::accumulate_request{2});
    REQUIRE(counter == 3);
}
