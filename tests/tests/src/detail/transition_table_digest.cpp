//Copyright Florian Goujeon 2021 - 2024.
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

    constexpr auto make_transition_table()
    {
        return maki::transition_table{}
        (state0,    maki::type<event0>, state1)
        (state1,    maki::type<event1>, state2, maki::null, guard0)
        (state2,    maki::type<event2>, state3, action0)
        (state3,    maki::type<event3>, state0, action1,    guard1)
        (maki::any, maki::type<event3>, state0)
        ;
    }

    constexpr auto transition_tuple = maki::detail::rows(make_transition_table());

    using transition_constant_list = maki::detail::tuple_to_constant_list_t<transition_tuple>;

    using digest_t = maki::detail::transition_table_digest
    <
        transition_constant_list
    >;

    using state_conf_ptr_constant_list = maki::detail::type_list
    <
        maki::detail::constant_t<&state0>,
        maki::detail::constant_t<&state1>,
        maki::detail::constant_t<&state2>,
        maki::detail::constant_t<&state3>
    >;
}

TEST_CASE("detail::transition_table_digest")
{
    using namespace transition_table_digest_ns;
    REQUIRE(std::is_same_v<digest_t::state_conf_ptr_constant_list, state_conf_ptr_constant_list>);
    REQUIRE(!digest_t::has_null_events);
}
