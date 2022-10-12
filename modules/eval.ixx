#include "../include/types.hpp"
#include "../include/macros.hpp"

#include <functional>
#include <variant>

export module Eval;
import Lib;

export namespace Eval
{
export enum class Operator
{
    Func,
    Add,
    Sub,
    Div,
    Mul,
    None
};

struct OperatorS
{
    // References the original string
    const u8 *begin = nullptr;
    const u8 *end   = nullptr;
};

template <typename Callable, typename... Args> auto ApplyFunc(Callable &&c, Args... args)
{
    return std::invoke(c, args...);
}

export enum class DataTypeTag
{
    List = 0,
    Int,
    Real,
    Ptr,
    None
};

export struct InternDataType
{
    DataTypeTag tag; // Tag isn't required if variants are used

    // std::variant<std::monostate, std::vector<InternDataType>, i64, f64, usize> data; // use visitor patterns for this
    union
    {
        i64                       integer;
        f64                       real = 0.0f;
        usize                     ptr;
        lib::List<InternDataType> list;
    } data;
};

struct Expression
{
    Expression()                   = default;

    bool                      leaf = false;
    OperatorS                 op; // if there are multiple types apply it recursively
    InternDataType            val;
    std::vector<Expression *> childs;
};

template <typename T, typename Callable>
requires std::is_invocable_v<Callable, T>
auto Transform(std::vector<T> &vec, Callable &&op) -> auto
{
    using U = std::invoke_result<Callable, T>::type;
    std::vector<U> nvec;
    nvec.reserve(vec.size());
    for (auto &v : vec)
        nvec.push_back(op(v));

    return std::move(nvec);
}

InternDataType EvaluateExpressionTree(Expression *expr);

struct InternVisitor
{
    // InternDataType operator()(auto &&callable, i64 x, i64 y)
    //{
    //     return InternDataType
    //     {
    //         DataTypeTag::None, callable(x, y);
    //     }
    // }
    // InternDataType operator()(auto &&callable, auto &x, auto &y)
    //{
    //     Assert("Error in the code");
    //     return InternDataType{};
    // }
};

export InternDataType ApplyOpIntern(InternDataType &x1, InternDataType &x2, auto &&callable, DataTypeTag tag)
{
    using enum DataTypeTag;
    switch (tag)
    {
    case Int:
        return InternDataType{tag, callable(x1.data.integer, x2.data.integer)};
    case Real:
    {
        auto data      = InternDataType{.tag = tag};
        data.data.real = callable(x1.data.real, x2.data.real);
        // return InternDataType{.tag = tag,.data =  callable(x1.data.real, x2.data.real)};
        return data;
    }
    default:
        Unimplemented();
    }
}

InternDataType AddExpressions(std::vector<Expression *> &childs)
{
    auto           data   = Transform(childs, EvaluateExpressionTree);

    InternDataType result = {DataTypeTag::None, {}};

    for (auto &x : data)
    {
        result = ApplyOpIntern(
            result, x, [](auto x, auto y) { return x + y; }, x.tag);
    }
    return result;
}

InternDataType SubExpressions(std::vector<Expression *> &childs)
{
    auto data = Transform(childs, EvaluateExpressionTree);

    Assert(data.size() > 1);
    InternDataType result = data[0];

    /*for (auto &x : data)
    {
        result = ApplyOpIntern(
            result, x, [](auto x, auto y) { return x - y; }, x.tag);
    }*/
    for (auto it = data.begin() + 1; it != data.end(); ++it)
    {
        result = ApplyOpIntern(
            result, *it, [](auto x, auto y) { return x - y; }, result.tag);
    }
    return result;
}

#define UNPACK_STR(str) (const u8 *)str, sizeof(str) - 1

InternDataType EvaluateLeaf(Expression *expr)
{
    return expr->val;
}

InternDataType EvaluateExpressionTree(Expression *expr)
{
    // if (expr->op == Operator::None)
    //     return expr->val;
    // switch (expr->op)
    //{
    // case Operator::Add:
    //     return AddExpressions(expr->childs);
    // case Operator::Sub:
    //     return SubExpressions(expr->childs);
    // default:
    //     Unimplemented();
    // }
    if (expr->leaf)
        return EvaluateLeaf(expr);

    usize len = expr->op.begin - expr->op.begin;

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("+")))
        return AddExpressions(expr->childs);

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("-")))
        return SubExpressions(expr->childs);

    Unimplemented();
}
} // namespace Eval

export void MyFunc();