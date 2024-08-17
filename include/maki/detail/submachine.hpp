//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_SUBMACHINE_HPP
#define MAKI_DETAIL_SUBMACHINE_HPP

#include "call_member.hpp"
#include "tlu.hpp"
#include "region.hpp"
#include "maybe_bool_util.hpp"
#include "simple_state.hpp"
#include "submachine_no_context.hpp"
#include "state_traits.hpp"
#include "tuple.hpp"
#include "../machine_fwd.hpp"
#include "../state_conf.hpp"
#include "../machine_conf.hpp"
#include "../transition_table.hpp"
#include "../path.hpp"
#include "../filters.hpp"
#include <type_traits>

namespace maki::detail
{

template<auto Id, class ParentRegion>
class submachine
{
public:
    static constexpr auto identifier = Id;
    static constexpr const auto& conf = *Id;
    using conf_type = std::decay_t<decltype(conf)>;
    using option_set_type = std::decay_t<decltype(opts(conf))>;
    using transition_table_type_list = decltype(opts(conf).transition_tables);
    using context_type = typename option_set_type::context_type;

    template
    <
        class Machine,
        class ParentContext,
        class ConfType = conf_type,
        std::enable_if_t<!is_root_sm_conf_v<ConfType>, bool> = true
    >
    submachine(Machine& mach, ParentContext& parent_ctx):
        ctx_holder_(mach, parent_ctx),
        impl_(mach, context())
    {
    }

    auto& context()
    {
        return ctx_holder_.get();
    }

    const auto& context() const
    {
        return ctx_holder_.get();
    }

    template<class ParentContext>
    auto& context_or(ParentContext& /*parent_ctx*/)
    {
        return context();
    }

    template<class ParentContext>
    const auto& context_or(ParentContext& /*parent_ctx*/) const
    {
        return context();
    }

    template<const auto& StatePath, class ParentContext>
    auto& context_or(ParentContext& /*parent_ctx*/)
    {
        if constexpr(StatePath.empty())
        {
            return context();
        }
        else
        {
            return impl_.template context_or<StatePath>(context());
        }
    }

    template<const auto& StatePath, class ParentContext>
    const auto& context_or(ParentContext& /*parent_ctx*/) const
    {
        if constexpr(StatePath.empty())
        {
            return context();
        }
        else
        {
            return impl_.template context_or<StatePath>(context());
        }
    }

    template<const auto& StateRegionPath, const auto& StateConf>
    [[nodiscard]] bool active_state() const
    {
        return impl_.template active_state<StateRegionPath, StateConf>();
    }

    template<const auto& StateConf>
    [[nodiscard]] bool active_state() const
    {
        return impl_.template active_state<StateConf>();
    }

    template<const auto& RegionPath>
    [[nodiscard]] bool running() const
    {
        return impl_.template running<RegionPath>();
    }

    [[nodiscard]] bool running() const
    {
        return impl_.running();
    }

    template<class Machine, class ParentContext, class Event>
    void call_entry_action(Machine& mach, ParentContext& /*parent_ctx*/, const Event& event)
    {
        impl_.call_entry_action(mach, context(), event);
    }

    template<bool Dry = false, class Machine, class ParentContext, class Event>
    void call_internal_action
    (
        Machine& mach,
        ParentContext& /*parent_ctx*/,
        const Event& event
    )
    {
        impl_.call_internal_action(mach, context(), event);
    }

    template<bool Dry = false, class Machine, class ParentContext, class Event>
    void call_internal_action
    (
        Machine& mach,
        ParentContext& /*parent_ctx*/,
        const Event& event,
        bool& processed
    )
    {
        impl_.call_internal_action(mach, context(), event, processed);
    }

    template<bool Dry = false, class Machine, class ParentContext, class Event>
    void call_internal_action
    (
        Machine& mach,
        ParentContext& /*parent_ctx*/,
        const Event& event
    ) const
    {
        impl_.call_internal_action(mach, context(), event);
    }

    template<bool Dry = false, class Machine, class ParentContext, class Event>
    void call_internal_action
    (
        Machine& mach,
        ParentContext& /*parent_ctx*/,
        const Event& event,
        bool& processed
    ) const
    {
        impl_.call_internal_action(mach, context(), event, processed);
    }

    template<class Machine, class ParentContext, class Event>
    void call_exit_action
    (
        Machine& mach,
        ParentContext& /*parent_ctx*/,
        const Event& event
    )
    {
        impl_.call_exit_action(mach, context(), event);
    }

    template<class /*Event*/>
    static constexpr bool has_internal_action_for_event()
    {
        return true;
    }

private:
    using impl_type = submachine_no_context<identifier, ParentRegion>;

    context_holder<context_type, opts(conf).context_sig> ctx_holder_;
    impl_type impl_;
};

} //namespace

#endif
