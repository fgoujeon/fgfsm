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
#include "submachine_fwd.hpp"
#include "state_traits.hpp"
#include "tuple.hpp"
#include "../machine_fwd.hpp"
#include "../state_conf.hpp"
#include "../machine_conf.hpp"
#include "../transition_table.hpp"
#include "../path.hpp"
#include "../type_patterns.hpp"
#include <type_traits>

namespace maki::detail
{

template
<
    class ParentSm,
    class RegionIndexSequence
>
struct region_tuple;

template
<
    class ParentSm,
    int... RegionIndexes
>
struct region_tuple
<
    ParentSm,
    std::integer_sequence<int, RegionIndexes...>
>
{
    using type = tuple
    <
        region
        <
            ParentSm,
            RegionIndexes
        >...
    >;
};

template<const auto& Conf, class Parent>
class submachine
{
public:
    using conf_type = std::decay_t<decltype(Conf)>;
    using option_set_type = std::decay_t<decltype(opts(Conf))>;
    using transition_table_type_list = decltype(opts(Conf).transition_tables);

    template
    <
        class Machine,
        class... ContextArgs,
        class ConfType = conf_type,
        std::enable_if_t<is_root_sm_conf_v<ConfType>, bool> = true
    >
    submachine(Machine& mach, ContextArgs&&... ctx_args):
        impl_(mach, std::forward<ContextArgs>(ctx_args)...),
        regions_(uniform_construct, mach, context())
    {
    }

    template
    <
        class Machine,
        class ParentContext,
        class ConfType = conf_type,
        std::enable_if_t<!is_root_sm_conf_v<ConfType>, bool> = true
    >
    submachine(Machine& mach, ParentContext& parent_ctx):
        impl_(mach, parent_ctx),
        regions_(uniform_construct, mach, context_or(parent_ctx))
    {
    }

    auto& context()
    {
        return impl_.context();
    }

    const auto& context() const
    {
        return impl_.context();
    }

    template<class ParentContext>
    auto& context_or(ParentContext& parent_ctx)
    {
        return impl_.context_or(parent_ctx);
    }

    template<class ParentContext>
    const auto& context_or(ParentContext& parent_ctx) const
    {
        return impl_.context_or(parent_ctx);
    }

    template<const auto& StatePath, class ParentContext>
    auto& context_or(ParentContext& parent_ctx)
    {
        if constexpr(StatePath.empty())
        {
            return context_or(parent_ctx);
        }
        else
        {
            static constexpr int region_index = path_raw_head(StatePath);
            static constexpr auto state_path_tail = path_tail(StatePath);
            return tuple_get<region_index>(regions_).template context_or<state_path_tail>(context_or(parent_ctx));
        }
    }

    template<const auto& StatePath, class ParentContext>
    const auto& context_or(ParentContext& parent_ctx) const
    {
        if constexpr(StatePath.empty())
        {
            return context_or(parent_ctx);
        }
        else
        {
            static constexpr int region_index = path_raw_head(StatePath);
            static constexpr auto state_path_tail = path_tail(StatePath);
            return tuple_get<region_index>(regions_).template context_or<state_path_tail>(context_or(parent_ctx));
        }
    }

    template<const auto& StateRegionPath, const auto& StateConf>
    [[nodiscard]] bool active_state() const
    {
        static constexpr auto region_index = path_raw_head(StateRegionPath);
        static constexpr auto state_region_relative_path = path_tail(StateRegionPath);
        return tuple_get<region_index>(regions_).template active_state<state_region_relative_path, &StateConf>();
    }

    template<const auto& StateConf>
    [[nodiscard]] bool active_state() const
    {
        static_assert(tlu::size_v<transition_table_type_list> == 1);

        static constexpr auto state_region_relative_path = path<>{};
        return tuple_get<0>(regions_).template active_state<state_region_relative_path, &StateConf>();
    }

    template<const auto& RegionPath>
    [[nodiscard]] bool running() const
    {
        return !active_state<RegionPath, state_confs::stopped>();
    }

    [[nodiscard]] bool running() const
    {
        return !active_state<state_confs::stopped>();
    }

    template<class Machine, class Event>
    void call_entry_action(Machine& mach, const Event& event)
    {
        impl_.call_entry_action(mach, event);
        tlu::for_each<region_tuple_type, region_start>(*this, mach, context(), event);
    }

    template<class Machine, class ParentContext, class Event>
    void call_entry_action(Machine& mach, ParentContext& parent_ctx, const Event& event)
    {
        impl_.call_entry_action(mach, context_or(parent_ctx), event);
        tlu::for_each<region_tuple_type, region_start>(*this, mach, context_or(parent_ctx), event);
    }

    template<bool Dry = false, class Machine, class Event>
    void call_internal_action
    (
        Machine& mach,
        const Event& event
    )
    {
        call_internal_action_2<Dry>(*this, mach, event);
    }

    template<bool Dry = false, class Machine, class Event>
    void call_internal_action
    (
        Machine& mach,
        const Event& event
    ) const
    {
        call_internal_action_2<Dry>(*this, mach, event);
    }

    template<bool Dry = false, class Machine, class ParentContext, class Event>
    void call_internal_action
    (
        Machine& mach,
        ParentContext& parent_ctx,
        const Event& event
    )
    {
        call_internal_action_2<Dry>(*this, mach, context_or(parent_ctx), event);
    }

    template<bool Dry = false, class Machine, class ParentContext, class Event>
    void call_internal_action
    (
        Machine& mach,
        ParentContext& parent_ctx,
        const Event& event
    ) const
    {
        call_internal_action_2<Dry>(*this, mach, context_or(parent_ctx), event);
    }

    template<bool Dry = false, class Machine, class Event>
    void call_internal_action
    (
        Machine& mach,
        const Event& event,
        bool& processed
    )
    {
        call_internal_action_2<Dry>(*this, mach, event, processed);
    }

    template<bool Dry = false, class Machine, class Event>
    void call_internal_action
    (
        Machine& mach,
        const Event& event,
        bool& processed
    ) const
    {
        call_internal_action_2<Dry>(*this, mach, event, processed);
    }

    template<bool Dry = false, class Machine, class ParentContext, class Event>
    void call_internal_action
    (
        Machine& mach,
        ParentContext& parent_ctx,
        const Event& event,
        bool& processed
    )
    {
        call_internal_action_2<Dry>(*this, mach, context_or(parent_ctx), event, processed);
    }

    template<bool Dry = false, class Machine, class ParentContext, class Event>
    void call_internal_action
    (
        Machine& mach,
        ParentContext& parent_ctx,
        const Event& event,
        bool& processed
    ) const
    {
        call_internal_action_2<Dry>(*this, mach, context_or(parent_ctx), event, processed);
    }

    template<class Machine, class Event>
    void call_exit_action(Machine& mach, const Event& event)
    {
        tlu::for_each<region_tuple_type, region_stop>(*this, mach, context(), event);
        impl_.call_exit_action
        (
            mach,
            event
        );
    }

    template<class Machine, class ParentContext, class Event>
    void call_exit_action(Machine& mach, ParentContext& parent_ctx, const Event& event)
    {
        tlu::for_each<region_tuple_type, region_stop>(*this, mach, context_or(parent_ctx), event);
        impl_.call_exit_action
        (
            mach,
            context_or(parent_ctx),
            event
        );
    }

    template<class /*Event*/>
    static constexpr bool has_internal_action_for_event()
    {
        return true;
    }

    static constexpr const auto& conf = Conf;

private:
    using region_tuple_type = typename region_tuple
    <
        submachine,
        std::make_integer_sequence<int, tlu::size_v<transition_table_type_list>>
    >::type;

    //We need a simple_state to manage context and actions
    using simple_state_type = simple_state<Conf>;

    struct region_start
    {
        template<class Region, class Self, class Machine, class Context, class Event>
        static void call(Self& self, Machine& mach, Context& ctx, const Event& event)
        {
            tuple_get<Region>(self.regions_).start(mach, ctx, event);
        }
    };

    template<bool Dry>
    struct region_process_event
    {
        template<class Region, class Self, class Machine, class Context, class Event, class... MaybeBool>
        static void call(Self& self, Machine& mach, Context& ctx, const Event& event, MaybeBool&... processed)
        {
            tuple_get<Region>(self.regions_).template process_event<Dry>(mach, ctx, event, processed...);
        }
    };

    struct region_stop
    {
        template<class Region, class Self, class Machine, class Context, class Event>
        static void call(Self& self, Machine& mach, Context& ctx, const Event& event)
        {
            tuple_get<Region>(self.regions_).stop(mach, ctx, event);
        }
    };

    template<bool Dry, class Self, class Machine, class Event, class... MaybeBool>
    static void call_internal_action_2
    (
        Self& self,
        Machine& mach,
        const Event& event,
        MaybeBool&... processed
    )
    {
        if constexpr(simple_state_type::template has_internal_action_for_event<Event>())
        {
            if constexpr(!Dry)
            {
                self.impl_.call_internal_action
                (
                    mach,
                    event
                );

                tlu::for_each<region_tuple_type, region_process_event<Dry>>(self, mach, self.context(), event);
            }

            maybe_bool_util::set_to_true(processed...);
        }
        else
        {
            tlu::for_each<region_tuple_type, region_process_event<Dry>>(self, mach, self.context(), event, processed...);
        }
    }

    template<bool Dry, class Self, class Machine, class Context, class Event, class... MaybeBool>
    static void call_internal_action_2
    (
        Self& self,
        Machine& mach,
        Context& ctx,
        const Event& event,
        MaybeBool&... processed
    )
    {
        if constexpr(simple_state_type::template has_internal_action_for_event<Event>())
        {
            if constexpr(!Dry)
            {
                self.impl_.call_internal_action
                (
                    mach,
                    ctx,
                    event
                );

                tlu::for_each<region_tuple_type, region_process_event<Dry>>(self, mach, ctx, event);
            }

            maybe_bool_util::set_to_true(processed...);
        }
        else
        {
            tlu::for_each<region_tuple_type, region_process_event<Dry>>(self, mach, ctx, event, processed...);
        }
    }

    simple_state_type impl_;
    region_tuple_type regions_;
};

} //namespace

#endif
