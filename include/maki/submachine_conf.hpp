//Copyright Florian Goujeon 2021 - 2024.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the maki::submachine_conf struct template
*/

#ifndef MAKI_SUBMACHINE_CONF_HPP
#define MAKI_SUBMACHINE_CONF_HPP

#include "transition_table.hpp"
#include "type_patterns.hpp"
#include "type.hpp"
#include "detail/context_signature.hpp"
#include "detail/event_action.hpp"
#include "detail/tuple.hpp"
#include "detail/type_list.hpp"
#include "detail/tlu.hpp"
#include "detail/tu.hpp"
#include <string_view>

namespace maki
{

namespace detail
{
    template
    <
        class Context = void,
        class ContextSigTag = context_signature_v_tag_t,
        class EntryActionTuple = detail::tuple<>,
        class InternalActionTuple = detail::tuple<>,
        class ExitActionTuple = detail::tuple<>,
        class TransitionTableTypeList = type_list<>
    >
    struct submachine_conf_option_set
    {
        using context_type = Context;
        using context_sig_tag_type = ContextSigTag;

        EntryActionTuple entry_actions;
        InternalActionTuple internal_actions;
        ExitActionTuple exit_actions;
        std::string_view pretty_name;
        TransitionTableTypeList transition_tables;
    };
}

#ifdef MAKI_DETAIL_DOXYGEN
/**
@brief Submachine configuration
*/
template<IMPLEMENTATION_DETAIL>
#else
template<class OptionSet = detail::submachine_conf_option_set<>>
#endif
class submachine_conf;

namespace detail
{
    //Read access to options
    template<class OptionSet>
    constexpr const auto& opts(const submachine_conf<OptionSet>& conf)
    {
        return conf.options_;
    }
}

#ifdef MAKI_DETAIL_DOXYGEN
template<IMPLEMENTATION_DETAIL>
#else
template<class OptionSet>
#endif
class submachine_conf
{
public:
    using context_type = typename OptionSet::context_type;

    constexpr submachine_conf() = default;

    submachine_conf(const submachine_conf&) = delete;

    submachine_conf(submachine_conf&&) = delete;

    ~submachine_conf() = default;

    submachine_conf& operator=(const submachine_conf&) = delete;

    submachine_conf& operator=(submachine_conf&&) = delete;

    constexpr bool operator==(const submachine_conf& other) const
    {
        return this == &other;
    }

#define MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_BEGIN /*NOLINT(cppcoreguidelines-macro-usage)*/ \
    [[maybe_unused]] const auto MAKI_DETAIL_ARG_context_type = type<typename OptionSet::context_type>; \
    [[maybe_unused]] const auto MAKI_DETAIL_ARG_context_sig_tag_type = type<typename OptionSet::context_sig_tag_type>; \
    [[maybe_unused]] const auto MAKI_DETAIL_ARG_entry_actions = options_.entry_actions; \
    [[maybe_unused]] const auto MAKI_DETAIL_ARG_internal_actions = options_.internal_actions; \
    [[maybe_unused]] const auto MAKI_DETAIL_ARG_exit_actions = options_.exit_actions; \
    [[maybe_unused]] const auto MAKI_DETAIL_ARG_pretty_name_view = options_.pretty_name; \
    [[maybe_unused]] const auto MAKI_DETAIL_ARG_transition_tables = options_.transition_tables;

#define MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_END /*NOLINT(cppcoreguidelines-macro-usage)*/ \
    return submachine_conf \
    < \
        detail::submachine_conf_option_set \
        < \
            typename std::decay_t<decltype(MAKI_DETAIL_ARG_context_type)>::type, \
            typename std::decay_t<decltype(MAKI_DETAIL_ARG_context_sig_tag_type)>::type, \
            std::decay_t<decltype(MAKI_DETAIL_ARG_entry_actions)>, \
            std::decay_t<decltype(MAKI_DETAIL_ARG_internal_actions)>, \
            std::decay_t<decltype(MAKI_DETAIL_ARG_exit_actions)>, \
            std::decay_t<decltype(MAKI_DETAIL_ARG_transition_tables)> \
        > \
    > \
    { \
        MAKI_DETAIL_ARG_entry_actions, \
        MAKI_DETAIL_ARG_internal_actions, \
        MAKI_DETAIL_ARG_exit_actions, \
        MAKI_DETAIL_ARG_pretty_name_view, \
        MAKI_DETAIL_ARG_transition_tables \
    };

#define MAKI_DETAIL_X(signature) /*NOLINT(cppcoreguidelines-macro-usage)*/ \
    template<class Context> \
    [[nodiscard]] constexpr auto context_##signature(const type_t<Context>& /*ignored*/) const \
    { \
        return context<Context, detail::context_signature_##signature##_tag_t>(); \
    }
    MAKI_DETAIL_CONTEXT_SIGNATURES_FOR_STATE
#undef MAKI_DETAIL_X

#define MAKI_DETAIL_X(signature) /*NOLINT(cppcoreguidelines-macro-usage)*/ \
    template<class EventFilter, class Action> \
    [[nodiscard]] constexpr auto entry_action_##signature(const EventFilter& event_filter, const Action& action) const \
    { \
        return entry_action<detail::event_action_signature::signature>(event_filter, action); \
    }
    MAKI_DETAIL_EVENT_ACTION_SIGNATURES
#undef MAKI_DETAIL_X

#define MAKI_DETAIL_X(signature) /*NOLINT(cppcoreguidelines-macro-usage)*/ \
    template<class EventFilter, class Action> \
    [[nodiscard]] constexpr auto internal_action_##signature(const EventFilter& event_filter, const Action& action) const \
    { \
        return internal_action<detail::event_action_signature::signature>(event_filter, action); \
    }
    MAKI_DETAIL_EVENT_ACTION_SIGNATURES
#undef MAKI_DETAIL_X

#define MAKI_DETAIL_X(signature) /*NOLINT(cppcoreguidelines-macro-usage)*/ \
    template<class EventFilter, class Action> \
    [[nodiscard]] constexpr auto exit_action_##signature(const EventFilter& event_filter, const Action& action) const \
    { \
        return exit_action<detail::event_action_signature::signature>(event_filter, action); \
    }
    MAKI_DETAIL_EVENT_ACTION_SIGNATURES
#undef MAKI_DETAIL_X

    [[nodiscard]] constexpr auto pretty_name(const std::string_view value) const
    {
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_BEGIN
#define MAKI_DETAIL_ARG_pretty_name_view value
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_END
#undef MAKI_DETAIL_ARG_pretty_name_view
    }

    template<class... TransitionTables>
    [[nodiscard]] constexpr auto transition_tables(const TransitionTables&... tables) const
    {
        const auto tpl = detail::tuple<TransitionTables...>{detail::distributed_construct, tables...};
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_BEGIN
#define MAKI_DETAIL_ARG_transition_tables tpl
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_END
#undef MAKI_DETAIL_ARG_transition_tables
    }

private:
    template<class OptionSet2>
    friend class submachine_conf;

    template<class OptionSet2>
    friend constexpr const auto& detail::opts(const submachine_conf<OptionSet2>& conf);

    template<class... Args>
    constexpr submachine_conf(Args&&... args):
        options_{std::forward<Args>(args)...}
    {
    }

    template<class Context, class ContextSigTag>
    [[nodiscard]] constexpr auto context() const
    {
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_BEGIN
#define MAKI_DETAIL_ARG_context_type type<Context>
#define MAKI_DETAIL_ARG_context_sig_tag_type type<ContextSigTag>
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_END
#undef MAKI_DETAIL_ARG_context_type
#undef MAKI_DETAIL_ARG_context_sig_tag_type
    }

    template<detail::event_action_signature Sig, class EventFilter, class Action>
    [[nodiscard]] constexpr auto entry_action(const EventFilter& event_filter, const Action& action) const
    {
        const auto new_entry_actions = detail::tu::append
        (
            options_.entry_actions,
            detail::make_event_action<Sig>(event_filter, action)
        );

        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_BEGIN
#define MAKI_DETAIL_ARG_entry_actions new_entry_actions
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_END
#undef MAKI_DETAIL_ARG_entry_actions
    }

    template<detail::event_action_signature Sig, class EventFilter, class Action>
    [[nodiscard]] constexpr auto internal_action(const EventFilter& event_filter, const Action& action) const
    {
        const auto new_internal_actions = detail::tu::append
        (
            options_.internal_actions,
            detail::make_event_action<Sig>(event_filter, action)
        );

        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_BEGIN
#define MAKI_DETAIL_ARG_internal_actions new_internal_actions
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_END
#undef MAKI_DETAIL_ARG_internal_actions
    }

    template<detail::event_action_signature Sig, class EventFilter, class Action>
    [[nodiscard]] constexpr auto exit_action(const EventFilter& event_filter, const Action& action) const
    {
        const auto new_exit_actions = detail::tu::append
        (
            options_.exit_actions,
            detail::make_event_action<Sig>(event_filter, action)
        );

        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_BEGIN
#define MAKI_DETAIL_ARG_exit_actions new_exit_actions
        MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_END
#undef MAKI_DETAIL_ARG_exit_actions
    }

#undef MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_END
#undef MAKI_DETAIL_MAKE_SUBMACHINE_CONF_COPY_BEGIN

    OptionSet options_;
};

namespace detail
{
    template<class T>
    struct is_submachine_conf
    {
        static constexpr auto value = false;
    };

    template<class... Ts>
    struct is_submachine_conf<submachine_conf<Ts...>>
    {
        static constexpr auto value = true;
    };

    template<class T>
    constexpr auto is_submachine_conf_v = is_submachine_conf<T>::value;
}

} //namespace

#endif
