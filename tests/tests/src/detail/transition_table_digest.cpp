//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include "../common.hpp"
#include <maki.hpp>

namespace transition_table_digest_ns
{
    struct context{};

    EMPTY_STATE(state0)
    EMPTY_STATE(state1)
    EMPTY_STATE(state2)
    EMPTY_STATE(state3)

    struct event0{};
    struct event1{};
    struct event2{};
    struct event3{};

    void action0(){}
    void action1(){}

    bool guard0(){return true;}
    bool guard1(){return true;}

    constexpr auto transition_table = maki::transition_table{}
        .add<state0,      event0, state1>()
        .add<state1,      event1, state2, maki::noop, guard0>()
        .add<state2,      event2, state3, action0>()
        .add<state3,      event3, state0, action1,    guard1>()
        .add<maki::any_c, event3, state0>()
    ;

    constexpr auto transition_tuple = maki::detail::rows(transition_table);

    using transition_constant_list = maki::detail::tuple_to_constant_list_t<transition_tuple>;

    using digest_t = maki::detail::transition_table_digest
    <
        transition_constant_list
    >;

    using state_conf_constant_list = maki::detail::type_list
    <
        maki::cref_constant<state0>,
        maki::cref_constant<state1>,
        maki::cref_constant<state2>,
        maki::cref_constant<state3>
    >;
}

TEST_CASE("detail::transition_table_digest")
{
    using namespace transition_table_digest_ns;
    REQUIRE(std::is_same_v<digest_t::state_conf_constant_list, state_conf_constant_list>);
    REQUIRE(!digest_t::has_null_events);
}
