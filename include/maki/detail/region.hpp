//Copyright Florian Goujeon 2021 - 2025.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_REGION_HPP
#define MAKI_DETAIL_REGION_HPP

#include "state_id_to_type.hpp"
#include "transition_table_digest.hpp"
#include "transition_table_filters.hpp"
#include "state_type_list_filters.hpp"
#include "equals.hpp"
#include "maybe_bool_util.hpp"
#include "tuple.hpp"
#include "constant.hpp"
#include "tlu/apply.hpp"
#include "tlu/empty.hpp"
#include "tlu/find.hpp"
#include "tlu/front.hpp"
#include "tlu/push_back.hpp"
#include "../states.hpp"
#include "../action.hpp"
#include "../guard.hpp"
#include "../path.hpp"
#include "../null.hpp"
#include "../state_conf.hpp"
#include "../state.hpp"
#include "../transition_table.hpp"
#include <type_traits>

namespace maki
{
    template<class Impl>
    class region;
}

namespace maki::detail
{

namespace region_detail
{
    inline constexpr auto final_state_index = -1;

    template<class StateIdConstantList, auto StateId>
    struct state_id_to_index
    {
        static constexpr auto value = tlu::find_v<StateIdConstantList, constant_t<StateId>>;
    };

    template<class StateIdConstantList>
    struct state_id_to_index<StateIdConstantList, &state_confs::init>
    {
        static constexpr auto value = final_state_index;
    };

    template<class StateIdConstantList>
    struct state_id_to_index<StateIdConstantList, &state_confs::fin>
    {
        static constexpr auto value = final_state_index;
    };

    template<class StateIdConstantList, auto StateId>
    inline constexpr auto state_id_to_index_v = state_id_to_index<StateIdConstantList, StateId>::value;
}

template<const auto& TransitionTable, const auto& Path>
class region
{
public:
    template<class Machine, class Context>
    region(const maki::region<region>* pitf, Machine& mach, Context& ctx):
        pitf_(pitf),
        states_(uniform_construct, mach, ctx)
    {
    }

    region(const region&) = delete;
    region(region&&) = delete;
    region& operator=(const region&) = delete;
    region& operator=(region&&) = delete;
    ~region() = default;

    template<const auto& StateConf>
    [[nodiscard]] bool is() const
    {
        if constexpr(is_state_set_v<std::decay_t<decltype(StateConf)>>)
        {
            return is_active_state_id_in_set<&StateConf>();
        }
        else
        {
            return is_active_state_id<&StateConf>();
        }
    }

    [[nodiscard]] bool running() const
    {
        return !is_active_state_id<&state_confs::fin>();
    }

    template<class Machine, class Context, class Event>
    void start(Machine& mach, Context& ctx, const Event& event)
    {
        try_processing_event_in_transition<false>::template call<constant_t<0>>
        (
            *this,
            mach,
            ctx,
            event
        );
    }

    template<class Machine, class Context, class Event>
    void stop(Machine& mach, Context& ctx, const Event& event)
    {
        if(running())
        {
            with_active_state_id<state_id_constant_list, stop_2>
            (
                *this,
                mach,
                ctx,
                event
            );
        }
    }

    template<bool Dry, class Machine, class Context, class Event>
    void process_event(Machine& mach, Context& ctx, const Event& event)
    {
        process_event_2<Dry>(*this, mach, ctx, event);
    }

    template<bool Dry, class Machine, class Context, class Event>
    void process_event(Machine& mach, Context& ctx, const Event& event, bool& processed)
    {
        process_event_2<Dry>(*this, mach, ctx, event, processed);
    }

    template<bool Dry, class Machine, class Context, class Event>
    void process_event(Machine& mach, Context& ctx, const Event& event) const
    {
        process_event_2<Dry>(*this, mach, ctx, event);
    }

    template<bool Dry, class Machine, class Context, class Event>
    void process_event(Machine& mach, Context& ctx, const Event& event, bool& processed) const
    {
        process_event_2<Dry>(*this, mach, ctx, event, processed);
    }

    template<const auto& StateConf>
    const auto& state() const
    {
        return state_id_to_obj<&StateConf>();
    }

    static const auto& path()
    {
        static const auto value = maki::path{Path};
        return value;
    }

private:
    static constexpr const auto& transition_table = TransitionTable;
    static constexpr auto transition_tuple = maki::detail::rows(transition_table);

    using transition_table_digest_type =
        transition_table_digest<transition_tuple>
    ;

    using state_id_constant_list = typename transition_table_digest_type::state_id_constant_list;

    template<class... StateIdConstants>
    using state_id_constant_pack_to_state_tuple_t = tuple
    <
        state_traits::state_id_to_type_t<StateIdConstants::value, Path>...
    >;

    using state_tuple_type = tlu::apply_t
    <
        state_id_constant_list,
        state_id_constant_pack_to_state_tuple_t
    >;

    static constexpr auto pinitial_state_conf = tlu::front_t<state_id_constant_list>::value;

    template<bool Dry, class Self, class Machine, class Context, class Event, class... MaybeBool>
    static void process_event_2
    (
        Self& self,
        Machine& mach,
        Context& ctx,
        const Event& event,
        [[maybe_unused]] MaybeBool&... processed
    )
    {
        //List the transitions whose event set contains `Event`
        using candidate_transition_index_constant_list = transition_table_filters::by_event_t
        <
            transition_tuple,
            Event
        >;

        //List the state types that require us to call their internal actions
        using candidate_state_type_list =
            state_type_list_filters::by_required_on_event_t
            <
                state_tuple_type,
                region,
                Event
            >
        ;

        constexpr auto must_try_processing_event_in_transitions = !tlu::empty_v<candidate_transition_index_constant_list>;
        constexpr auto must_try_processing_event_in_active_state = !tlu::empty_v<candidate_state_type_list>;

        if constexpr(must_try_processing_event_in_transitions && must_try_processing_event_in_active_state)
        {
            /*
            There is a possibility of conflicting transition in this case.
            Note that nested transitions take precedence over higher-order
            transitions.
            */
            auto processed_in_active_state = false;
            try_processing_event_in_active_state<candidate_state_type_list, Dry>(self, mach, ctx, event, processed_in_active_state);
            if(processed_in_active_state)
            {
                maybe_bool_util::set_to_true(processed...);
            }
            else
            {
                try_processing_event_in_transitions<candidate_transition_index_constant_list, Dry>(self, mach, ctx, event, processed...);
            }
        }
        else if constexpr(!must_try_processing_event_in_transitions && must_try_processing_event_in_active_state)
        {
            try_processing_event_in_active_state<candidate_state_type_list, Dry>(self, mach, ctx, event, processed...);
        }
        else if constexpr(must_try_processing_event_in_transitions && !must_try_processing_event_in_active_state)
        {
            try_processing_event_in_transitions<candidate_transition_index_constant_list, Dry>(self, mach, ctx, event, processed...);
        }
    }

    struct stop_2
    {
        template<class ActiveStateIdConstant, class Machine, class Context, class Event>
        static void call(region& self, Machine& mach, Context& ctx, const Event& event)
        {
            self.process_event_in_transition
            <
                ActiveStateIdConstant::value,
                &state_confs::fin,
                &null
            >(mach, ctx, event);
        }
    };

    template<class TransitionIndexConstantList, bool Dry = false, class Self, class Machine, class Context, class Event, class... ExtraArgs>
    static void try_processing_event_in_transitions(Self& self, Machine& mach, Context& ctx, const Event& event, ExtraArgs&... extra_args)
    {
        tlu::for_each_or
        <
            TransitionIndexConstantList,
            try_processing_event_in_transition<Dry>
        >(self, mach, ctx, event, extra_args...);
    }

    //Check active state and guard
    template<bool Dry>
    struct try_processing_event_in_transition
    {
        template<class TransitionIndexConstant, class Self, class Machine, class Context, class Event, class... ExtraArgs>
        static bool call(Self& self, Machine& mach, Context& ctx, const Event& event, ExtraArgs&... extra_args)
        {
            static constexpr const auto& trans = tuple_get<TransitionIndexConstant::value>(transition_tuple);
            static constexpr auto source_state_conf = trans.source_state_conf;
            static constexpr auto action = trans.act;
            static constexpr auto guard = trans.grd;

            if constexpr(is_state_set_v<std::decay_t<decltype(source_state_conf)>>)
            {
                //List of state confs that belong to the source state set
                using matching_state_conf_constant_list = state_type_list_filters::by_state_set_t
                <
                    state_id_constant_list,
                    &source_state_conf
                >;

                static_assert(!tlu::empty_v<matching_state_conf_constant_list>);

                return tlu::for_each_or
                <
                    matching_state_conf_constant_list,
                    try_processing_event_in_transition_2
                    <
                        Dry,
                        trans.target_state_conf,
                        action,
                        guard
                    >
                >(self, mach, ctx, event, extra_args...);
            }
            else
            {
                return try_processing_event_in_transition_2
                <
                    Dry,
                    trans.target_state_conf,
                    action,
                    guard
                >::template call<constant_t<trans.source_state_conf>>
                (
                    self,
                    mach,
                    ctx,
                    event,
                    extra_args...
                );
            }
        }
    };

    template<bool Dry, auto TargetStateId, const auto& Action, const auto& Guard>
    struct try_processing_event_in_transition_2
    {
        template
        <
            class SourceStateIdConstant,
            class Self,
            class Machine,
            class Context,
            class Event,
            class... MaybeBool
        >
        static bool call
        (
            Self& self,
            Machine& mach,
            Context& ctx,
            const Event& event,
            MaybeBool&... processed
        )
        {
            //Make sure the transition source state is the active state
            if(!self.template is_active_state_id<SourceStateIdConstant::value>())
            {
                return false;
            }

            //Check guard
            if constexpr(!std::is_same_v<decltype(Guard), const null_t&>)
            {
                if(!detail::call_guard(Guard, ctx, mach, event))
                {
                    return false;
                }
            }

            if constexpr(!Dry)
            {
                self.template process_event_in_transition
                <
                    SourceStateIdConstant::value,
                    TargetStateId,
                    &Action
                >(mach, ctx, event);
            }

            maybe_bool_util::set_to_true(processed...);

            return true;
        }
    };

    template
    <
        auto SourceStateId,
        auto TargetStateId,
        auto ActionPtr,
        class Machine,
        class Context,
        class Event
    >
    void process_event_in_transition
    (
        Machine& mach,
        Context& ctx,
        const Event& event
    )
    {
        using machine_option_set_type = typename Machine::option_set_type;

        constexpr auto is_internal_transition = std::is_same_v
        <
            std::decay_t<decltype(TargetStateId)>,
            null_t
        >;

        if constexpr(!is_internal_transition)
        {
            if constexpr(!std::is_same_v<typename machine_option_set_type::pre_external_transition_hook_type, null_t>)
            {
                impl_of(Machine::conf).pre_external_transition_hook
                (
                    ctx,
                    *pitf_,
                    state_id_to_obj<SourceStateId>(),
                    event,
                    state_id_to_obj<TargetStateId>()
                );
            }

            if constexpr(!ptr_equals(SourceStateId, &state_confs::init))
            {
                auto& stt = state_id_to_obj<SourceStateId>();
                impl_of(stt).call_exit_action
                (
                    mach,
                    ctx,
                    event
                );
            }

            active_state_index_ = region_detail::state_id_to_index_v
            <
                state_id_constant_list,
                TargetStateId
            >;
        }

        if constexpr(!std::is_same_v<decltype(ActionPtr), const null_t*>)
        {
            detail::call_action
            (
                *ActionPtr,
                ctx,
                mach,
                event
            );
        }

        if constexpr(!is_internal_transition)
        {
            if constexpr(!ptr_equals(TargetStateId, &state_confs::fin))
            {
                auto& stt = state_id_to_obj<TargetStateId>();
                impl_of(stt).call_entry_action
                (
                    mach,
                    ctx,
                    event
                );
            }

            if constexpr(!std::is_same_v<typename machine_option_set_type::post_external_transition_hook_type, null_t>)
            {
                impl_of(Machine::conf).post_external_transition_hook
                (
                    ctx,
                    *pitf_,
                    state_id_to_obj<SourceStateId>(),
                    event,
                    state_id_to_obj<TargetStateId>()
                );
            }

            //Completion transition
            {
                using candidate_transition_index_constant_list = transition_table_filters::by_source_state_and_null_event_t
                <
                    transition_tuple,
                    TargetStateId
                >;

                if constexpr(!tlu::empty_v<candidate_transition_index_constant_list>)
                {
                    try_processing_event_in_transitions<candidate_transition_index_constant_list>(*this, mach, ctx, null);
                }
            }
        }
    }

    /*
    Call active_state.on_event(event)
    */
    template<class StateTypeList, bool Dry, class Self, class Machine, class Context, class Event, class... ExtraArgs>
    static void try_processing_event_in_active_state
    (
        Self& self,
        Machine& mach,
        Context& ctx,
        const Event& event,
        ExtraArgs&... extra_args
    )
    {
        tlu::for_each_or
        <
            StateTypeList,
            try_processing_event_in_active_state_2<Dry>
        >(self, mach, ctx, event, extra_args...);
    }

    template<bool Dry>
    struct try_processing_event_in_active_state_2
    {
        template<class State, class Self, class Machine, class Context, class Event, class... ExtraArgs>
        static bool call
        (
            Self& self,
            Machine& mach,
            Context& ctx,
            const Event& event,
            ExtraArgs&... extra_args
        )
        {
            if(!self.template is_active_state_type<State>())
            {
                return false;
            }

            impl_of(self.template state_type_to_obj<State>()).template call_internal_action<Dry>
            (
                mach,
                ctx,
                event,
                extra_args...
            );

            return true;
        }
    };

    template<class State>
    [[nodiscard]] bool is_active_state_type() const
    {
        return is_active_state_id<impl_of_t<State>::identifier>();
    }

    template<auto StateId>
    [[nodiscard]] bool is_active_state_id() const
    {
        constexpr auto given_state_index = region_detail::state_id_to_index_v
        <
            state_id_constant_list,
            StateId
        >;
        return given_state_index == active_state_index_;
    }

    template<auto StateSetPtr>
    [[nodiscard]] bool is_active_state_id_in_set() const
    {
        auto matches = false;
        with_active_state_id
        <
            tlu::push_back_t<state_id_constant_list, constant_t<&state_confs::fin>>,
            is_active_state_id_in_set_2<StateSetPtr>
        >(matches);
        return matches;
    }

    template<auto StateSetPtr>
    struct is_active_state_id_in_set_2
    {
        template<class ActiveStateIdConstant>
        static void call([[maybe_unused]] bool& matches)
        {
            if constexpr(contains(impl_of(*StateSetPtr), ActiveStateIdConstant::value))
            {
                matches = true;
            }
        }
    };

    template<class StateIdConstantList, class F, class... Args>
    void with_active_state_id(Args&&... args) const
    {
        tlu::for_each_or
        <
            StateIdConstantList,
            with_active_state_id_2<F>
        >(*this, std::forward<Args>(args)...);
    }

    template<class F>
    struct with_active_state_id_2
    {
        template<class StateIdConstant, class... Args>
        static bool call(const region& self, Args&&... args)
        {
            if(self.is_active_state_id<StateIdConstant::value>())
            {
                F::template call<StateIdConstant>(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }
    };

    template<class State>
    auto& state_type_to_obj()
    {
        return static_state_type_to_obj<State>(*this);
    }

    template<class State>
    const auto& state_type_to_obj() const
    {
        return static_state_type_to_obj<State>(*this);
    }

    template<auto StateId>
    auto& state_id_to_obj()
    {
        return static_state_id_to_obj<StateId>(*this);
    }

    template<auto StateId>
    const auto& state_id_to_obj() const
    {
        return static_state_id_to_obj<StateId>(*this);
    }

    //Note: We use static to factorize const and non-const Region
    template<class State, class Region>
    static auto& static_state_type_to_obj(Region& self)
    {
        return static_state_id_to_obj<impl_of_t<State>::identifier>(self);
    }

    //Note: We use static to factorize const and non-const Region
    template<auto StateId, class Region>
    static auto& static_state_id_to_obj(Region& self)
    {
        if constexpr(ptr_equals(StateId, &state_confs::init))
        {
            return states::init;
        }
        else if constexpr(ptr_equals(StateId, &state_confs::fin))
        {
            return states::fin;
        }
        else
        {
            static constexpr auto state_index =
                region_detail::state_id_to_index_v
                <
                    state_id_constant_list,
                    StateId
                >
            ;
            return tuple_get<state_index>(self.states_);
        }
    }

    const maki::region<region>* pitf_;
    state_tuple_type states_;
    int active_state_index_ = region_detail::final_state_index;
};

} //namespace

#endif
