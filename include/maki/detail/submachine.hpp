//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_SUBMACHINE_HPP
#define MAKI_DETAIL_SUBMACHINE_HPP

#include "call_member.hpp"
#include "tlu.hpp"
#include "region.hpp"
#include "machine_object_holder.hpp"
#include "context_holder.hpp"
#include "submachine_fwd.hpp"
#include "tuple.hpp"
#include "../machine_fwd.hpp"
#include "../state_conf.hpp"
#include "../transition_table.hpp"
#include "../region_path.hpp"
#include "../type_patterns.hpp"
#include <type_traits>

namespace maki::detail
{

template<class ConfHolder, class ParentRegion>
struct machine_of<submachine<ConfHolder, ParentRegion>>
{
    using type = root_sm_of_t<ParentRegion>;

    static type& get(submachine<ConfHolder, ParentRegion>& node)
    {
        return node.root_sm();
    }
};

template<class ConfHolder>
struct machine_of<submachine<ConfHolder, void>>
{
    using type = machine<ConfHolder>;

    static type& get(submachine<ConfHolder, void>& node)
    {
        return node.root_sm();
    }
};

template<class ConfHolder, class ParentRegion>
struct submachine_context
{
    using conf_type = std::decay_t<decltype(ConfHolder::conf)>;
    using conf_context_type = typename conf_type::context_type;

    /*
    Context type is either (in this order of priority):
    - the one specified in the submachine_opts::context option, if any;
    - a reference to the context type of the parent SM (not necessarily the root
      SM).
    */
    using type = std::conditional_t
    <
        std::is_void_v<conf_context_type>,
        typename ParentRegion::parent_sm_type::context_type&,
        conf_context_type
    >;
};

template<class ConfHolder>
struct submachine_context<ConfHolder, void>
{
    using type = typename decltype(ConfHolder::conf.context_)::type;
};

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

template<class ConfHolder, class ParentRegion>
class submachine
{
public:
    using conf_holder_type = ConfHolder;
    using conf_type = std::decay_t<decltype(ConfHolder::conf)>;
    using data_type = std::conditional_t
    <
        std::is_void_v<typename conf_type::data_type>,
        null_t,
        typename conf_type::data_type
    >;

    using context_type = typename submachine_context<ConfHolder, ParentRegion>::type;
    using root_sm_type = root_sm_of_t<submachine>;

    using transition_table_type_list = decltype(ConfHolder::conf.transition_tables_);

    template<class... ContextArgs>
    submachine(root_sm_type& root_sm, ContextArgs&&... ctx_args):
        root_sm_(root_sm),
        ctx_holder_(root_sm, std::forward<ContextArgs>(ctx_args)...),
        data_holder_(root_sm, context()),
        regions_(uniform_construct, *this)
    {
    }

    root_sm_type& root_sm()
    {
        return root_sm_;
    }

    context_type& context()
    {
        return ctx_holder_.get();
    }

    data_type& data()
    {
        return data_holder_.get();
    }

    const data_type& data() const
    {
        return data_holder_.get();
    }

    template<const auto& StateRegionPath, const auto& StateConf>
    auto& state_def_data()
    {
        using state_region_path_t = std::decay_t<decltype(StateRegionPath)>;

        static_assert
        (
            same_ref
            (
                detail::tlu::front_t<state_region_path_t>::machine_conf,
                ConfHolder::conf
            )
        );

        static constexpr auto region_index = tlu::front_t<state_region_path_t>::region_index;
        static constexpr auto state_region_relative_path = tlu::pop_front_t<state_region_path_t>{};
        return tuple_get<region_index>(regions_).template state_def_data<state_region_relative_path, StateConf>();
    }

    template<const auto& StateRegionPath, const auto& StateConf>
    const auto& state_def_data() const
    {
        using state_region_path_t = std::decay_t<decltype(StateRegionPath)>;

        static_assert
        (
            same_ref
            (
                detail::tlu::front_t<state_region_path_t>::machine_conf,
                ConfHolder::conf
            )
        );

        static constexpr auto region_index = tlu::front_t<state_region_path_t>::region_index;
        static constexpr auto state_region_relative_path = tlu::pop_front_t<state_region_path_t>{};
        return tuple_get<region_index>(regions_).template state_def_data<state_region_relative_path, StateConf>();
    }

    template<const auto& StateRegionPath, const auto& StateConf>
    [[nodiscard]] bool is_active_state_def() const
    {
        using state_region_path_t = std::decay_t<decltype(StateRegionPath)>;

        static_assert
        (
            same_ref
            (
                detail::tlu::front_t<state_region_path_t>::machine_conf,
                ConfHolder::conf
            )
        );

        static constexpr auto region_index = tlu::front_t<state_region_path_t>::region_index;
        static constexpr auto state_region_relative_path = tlu::pop_front_t<state_region_path_t>{};
        return tuple_get<region_index>(regions_).template is_active_state_def<state_region_relative_path, StateConf>();
    }

    template<const auto& StateConf>
    [[nodiscard]] bool is_active_state_def() const
    {
        static_assert(tlu::size_v<transition_table_type_list> == 1);

        static constexpr auto state_region_relative_path = region_path<>{};
        return tuple_get<0>(regions_).template is_active_state_def<state_region_relative_path, StateConf>();
    }

    template<const auto& RegionPath>
    [[nodiscard]] bool is_running() const
    {
        return !is_active_state_def<RegionPath, states::stopped>();
    }

    [[nodiscard]] bool is_running() const
    {
        return !is_active_state_def<states::stopped>();
    }

    template<class Machine, class Context, class Event>
    void call_entry_action(Machine& mach, Context& /*ctx*/, const Event& event)
    {
        call_state_action
        (
            ConfHolder::conf.entry_actions_,
            mach,
            context(),
            data(),
            event
        );
        tlu::for_each<region_tuple_type, region_start>(*this, event);
    }

    template<class Event>
    void call_internal_action(const Event& event)
    {
        if constexpr(state_traits::requires_on_event_v<ConfHolder, Event>)
        {
            call_state_action
            (
                ConfHolder::conf.internal_actions_,
                root_sm_,
                context(),
                data(),
                event
            );
        }

        tlu::for_each<region_tuple_type, region_process_event>(*this, event);
    }

    template<class Event>
    void call_internal_action(const Event& event, bool& processed)
    {
        if constexpr(state_traits::requires_on_event_v<ConfHolder, Event>)
        {
            call_state_action
            (
                ConfHolder::conf.internal_actions_,
                root_sm_,
                context(),
                data(),
                event
            );
            tlu::for_each<region_tuple_type, region_process_event>(*this, event);
            processed = true;
        }
        else
        {
            tlu::for_each<region_tuple_type, region_process_event>(*this, event, processed);
        }
    }

    template<class Machine, class Context, class Event>
    void call_exit_action(Machine& mach, Context& /*ctx*/, const Event& event)
    {
        tlu::for_each<region_tuple_type, region_stop>(*this, event);
        call_state_action
        (
            ConfHolder::conf.exit_actions_,
            mach,
            context(),
            data(),
            event
        );
    }

    template<class /*Event*/>
    static constexpr bool has_internal_action_for_event()
    {
        return true;
    }

private:
    using region_tuple_type = typename region_tuple
    <
        submachine,
        std::make_integer_sequence<int, tlu::size_v<transition_table_type_list>>
    >::type;

    struct region_start
    {
        template<class Region, class Event>
        static void call(submachine& self, const Event& event)
        {
            tuple_get<Region>(self.regions_).start(event);
        }
    };

    struct region_process_event
    {
        template<class Region, class Event, class... ExtraArgs>
        static void call(submachine& self, const Event& event, ExtraArgs&... extra_args)
        {
            tuple_get<Region>(self.regions_).process_event(event, extra_args...);
        }
    };

    struct region_stop
    {
        template<class Region, class Event>
        static void call(submachine& self, const Event& event)
        {
            tuple_get<Region>(self.regions_).stop(event);
        }
    };

    //Store references for faster access
    root_sm_type& root_sm_; //NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

    context_holder<context_type> ctx_holder_;
    detail::machine_object_holder<data_type> data_holder_;
    region_tuple_type regions_;
};

} //namespace

#endif
