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
    };

    namespace states
    {
        struct on{};
        struct off{};
    }

    namespace events
    {
        struct on_button_press{};
        struct off_button_press{};
    }

    using sm_transition_table = awesm::transition_table
    <
        awesm::row<states::off, events::on_button_press,  states::on>,
        awesm::row<states::on,  events::off_button_press, states::off>
    >;

    struct sm_def
    {
        using conf = awesm::sm_conf
        <
            awesm::transition_table_list<sm_transition_table>,
            awesm::sm_options::disable_run_to_completion
        >;
    };

    using sm_t = awesm::sm<sm_def>;
}

TEST_CASE("sm_ref")
{
    using sm_ref_t =
        awesm::sm_ref<events::on_button_press, events::off_button_press>
    ;

    auto ctx = context{};
    auto sm = sm_t{ctx};
    auto psm_ref_temp = std::make_unique<sm_ref_t>(sm); //test ref of ref
    auto sm_ref = sm_ref_t{*psm_ref_temp};
    psm_ref_temp.reset();

    sm.start();

    REQUIRE(sm.is_active_state<states::off>());

    sm_ref.process_event(events::on_button_press{});
    REQUIRE(sm.is_active_state<states::on>());

    sm_ref.process_event(events::off_button_press{});
    REQUIRE(sm.is_active_state<states::off>());
}
