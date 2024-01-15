//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the maki::guard class template
*/

#ifndef MAKI_GUARD_HPP
#define MAKI_GUARD_HPP

#include "detail/storable_function.hpp"
#include "detail/call_member.hpp"
#include <type_traits>

namespace maki
{

namespace detail
{
    enum class guard_operator
    {
        none,
        not_,
        and_,
        or_,
        xor_
    };
}

template<detail::guard_operator Operator, class Operand, class Operand2 = null>
class guard;

namespace detail
{
    template<detail::guard_operator Operator, class Operand>
    constexpr auto make_guard(const Operand& operand)
    {
        return guard<Operator, Operand>{operand};
    }

    template<detail::guard_operator Operator, class Operand, class Operand2>
    constexpr auto make_guard(const Operand& operand, const Operand2& operand2)
    {
        return guard<Operator, Operand, Operand2>{operand, operand2};
    }
}

/**
@brief A guard wrapper that allows boolean composition
*/
template<detail::guard_operator Operator, class Operand, class Operand2>
class guard
{
public:
    constexpr guard(const Operand& operand):
        op_(operand)
    {
    }

    constexpr guard(const Operand& operand, const Operand2& operand2):
        op_(operand),
        op2_(operand2)
    {
    }

    constexpr guard(const guard& rhs) = default;

    guard(guard&& rhs) = delete;

    ~guard() = default;

    constexpr guard& operator=(const guard& rhs) = default;

    guard& operator=(guard&& rhs) = delete;

    template<class Sm, class Context, class Event>
    bool operator()(Sm& mach, Context& ctx, const Event& event) const
    {
        if constexpr(Operator == detail::guard_operator::none)
        {
            return detail::call_action_or_guard(op_, mach, ctx, event);
        }
        if constexpr(Operator == detail::guard_operator::not_)
        {
            return !op_(mach, ctx, event);
        }
        if constexpr(Operator == detail::guard_operator::and_)
        {
            return op_(mach, ctx, event) && op2_(mach, ctx, event);
        }
        if constexpr(Operator == detail::guard_operator::or_)
        {
            return op_(mach, ctx, event) || op2_(mach, ctx, event);
        }
        if constexpr(Operator == detail::guard_operator::xor_)
        {
            return op_(mach, ctx, event) != op2_(mach, ctx, event);
        }
    }

private:
    Operand op_;
    Operand2 op2_;
};

template<class Guard>
guard(const Guard&) -> guard<detail::guard_operator::none, detail::storable_function_t<Guard>>;

template
<
    detail::guard_operator LhsOperator, class LhsOperand, class LhsOperand2,
    detail::guard_operator RhsOperator, class RhsOperand, class RhsOperand2
>
constexpr auto operator&&
(
    const guard<LhsOperator, LhsOperand, LhsOperand2>& lhs,
    const guard<RhsOperator, RhsOperand, RhsOperand2>& rhs
)
{
    return detail::make_guard<detail::guard_operator::and_>(lhs, rhs);
}

template
<
    detail::guard_operator LhsOperator, class LhsOperand, class LhsOperand2,
    detail::guard_operator RhsOperator, class RhsOperand, class RhsOperand2
>
constexpr auto operator||
(
    const guard<LhsOperator, LhsOperand, LhsOperand2>& lhs,
    const guard<RhsOperator, RhsOperand, RhsOperand2>& rhs
)
{
    return detail::make_guard<detail::guard_operator::or_>(lhs, rhs);
}

template
<
    detail::guard_operator LhsOperator, class LhsOperand, class LhsOperand2,
    detail::guard_operator RhsOperator, class RhsOperand, class RhsOperand2
>
constexpr auto operator!=
(
    const guard<LhsOperator, LhsOperand, LhsOperand2>& lhs,
    const guard<RhsOperator, RhsOperand, RhsOperand2>& rhs
)
{
    return detail::make_guard<detail::guard_operator::xor_>(lhs, rhs);
}

template
<
    detail::guard_operator RhsOperator, class RhsOperand, class RhsOperand2
>
constexpr auto operator!
(
    const guard<RhsOperator, RhsOperand, RhsOperand2>& rhs
)
{
    return detail::make_guard<detail::guard_operator::not_>(rhs);
}

} //namespace

#endif
