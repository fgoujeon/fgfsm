//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_STATE_PROXY_FWD_HPP
#define MAKI_DETAIL_STATE_PROXY_FWD_HPP

namespace maki
{

template<class State>
class state_proxy;

namespace detail
{
    template<class State>
    state_proxy<State> make_state_proxy(const State& state);
}

} //namespace

#endif
