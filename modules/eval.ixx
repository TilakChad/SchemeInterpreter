#include "../include/types.hpp"
#include "../include/macros.hpp"

#include <functional>
#include <variant>
#include <iostream>
#include <ranges>

namespace ranges = std::ranges;

#define UNPACK_STR(str) (const u8 *)str, sizeof(str) - 1

import Log;
import Lib;

export module Eval;

export import :base;

export namespace Eval
{
template <typename Callable, typename... Args> auto ApplyFunc(Callable &&c, Args... args)
{
    return std::invoke(std::forward(c), args...);
}

ScopeStack       scopes;
GarbageCollector gc; // A minimalistic garbage collector implementing mark and sweep approach

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
Cons          *MakeCons(InternDataType &a, InternDataType &b);

export void    RunGarbageCollector()
{
    // Pop the last stack
    if (false /* last_ran*/)
        scopes.PopStack(); // Run the garbage collector for the last time, by popping the last stack
    gc.Run(&scopes);
    gc.FreeGarbage();
}

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
    InternDataType type;
    using enum DataTypeTag;
    switch (x1.tag)
    {
    case Int:

        if (x2.tag == Int)
            return {Int, callable(x1.data.integer, x2.data.integer)};
        if (x2.tag == Real)
            return {.tag = Real, .data = {.real = callable(x1.data.integer, x2.data.real)}};
        break;

    case Real:
    {
        if (x2.tag == Real)
            return InternDataType{.tag = tag, .data = {.real = callable(x1.data.real, x2.data.real)}};
        if (x2.tag == Int)
            return InternDataType{.tag = Real, .data = {.real = callable(x1.data.real, x2.data.integer)}};
    }
    default:
        Unimplemented();
    }
}

InternDataType AddExpressions(std::vector<Expression *> &childs)
{
    auto data = Transform(childs, EvaluateExpressionTree);

    Assert(data.size() > 1);

    InternDataType result = data[0];

    for (auto it = data.begin() + 1; it != data.end(); ++it)
    {
        result = ApplyOpIntern(
            result, *it, [](auto x, auto y) { return x + y; }, result.tag);
    }

    return result;
}

InternDataType ApplyLogicalAnd(Expression *expr)
{
    using enum DataTypeTag;
    // auto data = Transform(expr->childs, EvaluateExpressionTree);
    Assert(expr->childs.size());

    bool cond = true; // Empty conditions not allowed, do short  circuit evaluation --> which is strictly required

    for (auto &x : expr->childs)
    {
        auto val = EvaluateExpressionTree(x);
        cond     = cond && val.data.integer;
        if (!cond)
            break;
    }
    return {Int, cond};
}

InternDataType ApplyLogicalOr(Expression *expr)
{
    using enum DataTypeTag;
    Assert(expr->childs.size());

    bool cond = false; // Empty conditions not allowed, does short  circuit evaluation --> which is strictly required

    for (auto &x : expr->childs)
    {
        auto val = EvaluateExpressionTree(x);
        cond     = cond || val.data.integer;
        if (cond)
            break;
    }
    return {Int, cond};
}

InternDataType ApplyLogicalNot(Expression *expr)
{
    using enum DataTypeTag;
    Assert(expr->childs.size() == 1);
    return {Int, !EvaluateExpressionTree(expr->childs[0]).data.integer};
}

InternDataType ApplyComparisons(const InternDataType &x1, const InternDataType &x2, auto &&callable)
{
    using enum DataTypeTag;

    switch (x1.tag)
    {
    case Int:
    {
        if (x2.tag == Int)
            return {Int, callable(x1.data.integer, x2.data.integer)};
        if (x2.tag == Real)
            return {Int, callable(x1.data.integer, x2.data.real)};
    }
    break;

    case Real:
    {
        if (x2.tag == Real)
            return InternDataType{Int, callable(x1.data.real, x2.data.real)};
        if (x2.tag == Int)
            return InternDataType{Int, callable(x1.data.real, x2.data.integer)};
    }
    default:
        Unimplemented();
    }
}

InternDataType MultiplyExpressions(std::vector<Expression *> &childs)
{
    auto data = Transform(childs, EvaluateExpressionTree);

    Assert(data.size() > 1);

    InternDataType result = data[0];

    for (auto it = data.begin() + 1; it != data.end(); ++it)
    {
        result = ApplyOpIntern(
            result, *it, [](auto x, auto y) { return x * y; }, result.tag);
    }

    return result;
}

InternDataType DivideExpressions(std::vector<Expression *> &childs)
{
    auto data = Transform(childs, EvaluateExpressionTree);

    Assert(data.size() > 1);

    InternDataType result = data[0];

    for (auto it = data.begin() + 1; it != data.end(); ++it)
    {
        result = ApplyOpIntern(
            result, *it, [](auto x, auto y) { return x / y; }, result.tag);
    }

    return result;
}

InternDataType SubExpressions(std::vector<Expression *> &childs)
{
    auto data = Transform(childs, EvaluateExpressionTree);

    if (data.size() == 1)
    {
        switch (data[0].tag)
        {
        case DataTypeTag::Int:
            return InternDataType{data[0].tag, -data[0].data.integer};
        case DataTypeTag::Real:
            return InternDataType{.tag = data[0].tag, .data = {.real = -data[0].data.real}};
            Unimplemented();
        }
    }

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

InternDataType EvaluateLeaf(Expression *expr)
{
    using enum DataTypeTag;
    switch (expr->val.tag)
    {
    case Int:
    case Real:
    case Ptr:
        return expr->val;
    case Id:
    {
        auto current_scope = scopes.GetStackTop();
        // auto entry         = current_scope->GetEntryInTable(expr->val);
        auto entry = scopes.GetSymbolEntry(expr->val);
        Assert(entry != nullptr);

        return entry->data.var;
    }
    default:
        Unimplemented();
    }
}

// InternDataType ConstructList(Expression *expr)
//{
//     return InternDataType(DataTypeTag::List);
// }

InternDataType EvalUserDefinedFunction(Expression *expr, Funcs &func)
{
    using Type = SymbolTableEntry::EntryType;

    SymbolTable table;
    scopes.PushStack(table);
    // Enter the entry as specified in order
    auto             current_scope = scopes.GetStackTop();
    SymbolTableEntry entry;

    Assert(expr->childs.size() == func.args_count); // args count mismatch

    entry.entry_type = Type::Var;
    u32 index        = 0;

    // TODO :: Fix this function call to possibly support infinite data structures

    auto transformed = Transform(expr->childs, EvaluateExpressionTree);

    for (auto x : transformed)
    {
        entry.name     = func.args[index];
        entry.data.var = x;

        GetSingletonLogger().Log(std::string_view((const char *)entry.name.begin, 1), "  ", entry.data.var.data.integer,
                                 '\n');

        index = index + 1;
        current_scope->table.push_back(entry);
    }

    GetSingletonLogger().Log('\n');

    InternDataType retval = {DataTypeTag::None};
    // Open new scope -> already opened
    for (u32 index = 1; index < func.body->childs.size(); ++index)
        retval = EvaluateExpressionTree(func.body->childs[index]);

    // EvaluateExpressionTree(func.body->childs[1]);
    scopes.PopStack();
    // Return the last evaluated value
    return retval;
}

InternDataType EvaluateLambda(Expression *expr, InternDataType &val)
{
    Assert(val.tag == DataTypeTag::Lambda);

    // scopes.PushStack(*val.data.lambda.closure);
    // auto res = EvalUserDefinedFunction(expr, val.data.lambda.func);
    // scopes.PopStack();
    // return res;

    using Type         = SymbolTableEntry::EntryType;
    auto       &func   = val.data.lambda.func;
    auto const &lambda = val.data.lambda;
    // Above doesn't work with same lambda capture and args
    SymbolTable table;
    scopes.PushStack(table);
    // Enter the entry as specified in order
    auto             current_scope = scopes.GetStackTop();
    SymbolTableEntry entry;

    Assert(expr->childs.size() == func.args_count); // args count mismatch

    entry.entry_type = Type::Var;
    u32 index        = 0;

    // TODO :: Fix this function call to possibly support infinite data structures

    auto transformed = Transform(expr->childs, EvaluateExpressionTree);
    for (auto x : transformed)
    {
        entry.name     = func.args[index];
        entry.data.var = x;

        GetSingletonLogger().Log(std::string_view((const char *)entry.name.begin, 1), "  ", entry.data.var.data.integer,
                                 '\n');

        index = index + 1;
        current_scope->table.push_back(entry);
    }

    GetSingletonLogger().Log('\n');

    InternDataType retval = {DataTypeTag::None};

    // Open new scope for lambda evaluation
    scopes.PushStack(*lambda.closure);
    for (u32 index = 1; index < func.body->childs.size(); ++index)
        retval = EvaluateExpressionTree(func.body->childs[index]);

    // EvaluateExpressionTree(func.body->childs[1]);
    scopes.PopStack();
    scopes.PopStack();
    // Return the last evaluated value
    return retval;
}

InternDataType HandleUserDefinedFunctions(Expression *expr)
{
    InternDataType val = {.tag = DataTypeTag::Id};
    val.data.id        = expr->op;

    auto current_scope = scopes.GetStackTop();
    auto entry         = scopes.GetSymbolEntry(val);
    Assert(entry != nullptr);
    if (entry->entry_type == SymbolTableEntry::EntryType::Function)
        return EvalUserDefinedFunction(expr, entry->data.func);
    else
        return EvaluateLambda(expr, entry->data.var);
}

void CaptureClosureAST(Expression *expr, Lambda &lambda)
{
    if (expr->leaf)
    {
        if (expr->val.tag == DataTypeTag::Id)
        {
            GetSingletonLogger().Log("Leaf visited : ",
                                     std::string_view((const char *)expr->val.data.id.begin,
                                                      (u32)(expr->val.data.id.end - expr->val.data.id.begin + 1)),
                                     '\n');
            bool not_arg = true;

            for (u32 arg = 0; arg < lambda.func.args_count; ++arg)
            {
                u32 len = lambda.func.args[arg].end - lambda.func.args[arg].begin + 1;
                if (lib::StrCmpEqual(expr->val.data.id.begin, expr->val.data.id.end, lambda.func.args[arg].begin, len))
                {
                    not_arg = false;
                    break;
                }
            }
            if (not_arg)
            {
                GetSingletonLogger().Log("Variable captured by lambda : ",
                                         std::string_view((const char *)expr->val.data.id.begin,
                                                          (u32)(expr->val.data.id.end - expr->val.data.id.begin + 1)),
                                         "\n\n");
                // Find entry in the symbol table and push to the lambda symbol table,
                // symbol_table should be  behind the shared reference
                auto entry = scopes.GetSymbolEntry(expr->val);
                Assert(entry != nullptr);
                lambda.closure->table.push_back(*entry);
            }
        }

        return;
    }

    for (auto x : expr->childs)
        CaptureClosureAST(x, lambda);
}

// InternDataType ProcessLambdaBody(Expression *expr, Lambda &lambda)
//{
//     return
// }

// Let Over Lambda
InternDataType DefineLet(Expression *expr)
{
    scopes.PushStack(SymbolTable());

    auto             current_scope = scopes.GetStackTop();
    SymbolTableEntry entry;
    entry.entry_type = SymbolTableEntry::EntryType::Var;

    // It can only have two childs
    Assert(expr->childs.size() == 2);

    for (auto &var : expr->childs[0]->childs)
    {
        // Don't evaluate just push to the stack
        entry.name     = {var->op.end, var->op.begin};
        entry.data.var = EvaluateExpressionTree(var->childs[0]);
        current_scope->table.push_back(entry);
    }

    auto val = EvaluateExpressionTree(expr->childs[1]);
    return val;
}

InternDataType DefineLambda(Expression *expr)
{
    InternDataType val = {DataTypeTag::Lambda};

    Lambda         lambda;
    lambda.closure = new SymbolTable();
    gc.AddAllocation(lambda.closure, sizeof(SymbolTable));
    auto fn           = &lambda.func;
    lambda.name       = expr->op; // lambda have no names

    fn->args[0].begin = expr->childs[0]->op.begin;
    fn->args[0].end   = expr->childs[0]->op.end;

    fn->args_count    = fn->args_count + 1;
    for (auto const &param : expr->childs[0]->childs)
    {
        Assert(fn->args_count < fn->MAX_ARGS - 1);

        fn->args[fn->args_count].begin = param->val.data.id.begin;
        fn->args[fn->args_count].end   = param->val.data.id.end;

        fn->args_count                 = fn->args_count + 1;

        Assert(param->leaf);
        GetSingletonLogger().Log("\nLamda parameters are :  ",
                                 std::string_view((const char *)param->val.data.id.begin,
                                                  u32(param->val.data.id.end - param->val.data.id.begin + 1)));
    }

    // push all the segments as the body
    fn->body = expr; // Instead of  handling this a simple stack of captured variable pushed after the lambda would've
                     // beein nice
    // Step into the function body and evaluate
    // fn.body         = expr->childs[1];
    // Lambdas aren't pushed to any stacks
    CaptureClosureAST(expr->childs[1], lambda);
    val.data.lambda = lambda;
    return val;
}

Cons *MakeCons(InternDataType &x1, InternDataType &x2)
{
    // It also means that the pointers will be shallow copied
    // No garbage collection
    auto cons = new Cons{};
    gc.AddAllocation(cons, sizeof(Cons));
    cons->car = x1;
    cons->cdr = x2;
    return cons;
}

InternDataType DefineCons(Expression *expr)
{
    auto           x1   = EvaluateExpressionTree(expr->childs[0]);
    auto           x2   = EvaluateExpressionTree(expr->childs[1]);
    auto           cons = MakeCons(x1, x2);
    InternDataType val  = {DataTypeTag::Cons};
    val.data.cons       = cons;
    return val;
}

InternDataType Car(Expression *expr)
{
    auto x1 = EvaluateExpressionTree(expr->childs[0]);
    Assert(x1.tag == DataTypeTag::Cons);
    return x1.data.cons->car;
}

InternDataType Cdr(Expression *expr)
{
    auto x1 = EvaluateExpressionTree(expr->childs[0]);
    Assert(x1.tag == DataTypeTag::Cons);
    return x1.data.cons->cdr;
}

InternDataType HandleBuiltinFunctions(Expression *expr)
{
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("define")))
    {
        // Read  the next symbol its value and push to the symbol table stack
        // Get the first child and assign  the  value of the second child
        using Type                     = SymbolTableEntry::EntryType;

        auto             current_scope = scopes.GetStackTop();
        SymbolTableEntry entry;

        const auto      &f = expr->childs[0];
        if (f->val.tag == DataTypeTag::Id)
        {

            entry.entry_type = Type::Var;
            // Fully evaluate the second child

            auto s           = EvaluateExpressionTree(expr->childs[1]);
            entry.name       = f->val.data.id;

            entry.entry_type = Type::Var;
            entry.data.var   = s;

            current_scope->table.push_back(entry);
            current_scope->dump();
            return InternDataType{DataTypeTag::None};
        }
        //  This one  is too responsible for parsing functions too
        //  Parse  the function  and the body

        // First child contains the name of the functions and the formal parameters
        // Second child contains the definition  of the function

        // Shall we use evaulative style function calling or the replacement one?
        // Lets try the Scheme way substitution one, which is evaluated only when the functions are called not
        // when defined though

        Funcs fn;
        entry.entry_type = Type::Function;

        entry.name       = f->op;

        // Child of the first args tagged as id are the list of the formal parameters of this function

        for (auto const &param : f->childs)
        {
            Assert(fn.args_count < fn.MAX_ARGS - 1);

            fn.args[fn.args_count].begin = param->val.data.id.begin;
            fn.args[fn.args_count].end   = param->val.data.id.end;

            fn.args_count                = fn.args_count + 1;

            Assert(param->leaf);
            GetSingletonLogger().Log("\nParams are :  ",
                                     std::string_view((const char *)param->val.data.id.begin,
                                                      u32(param->val.data.id.end - param->val.data.id.begin + 1)));
        }

        // push all the segments as the body
        fn.body = expr; //  Remember : while evaluating  the function skip the first child  and continue
        // fn.body         = expr->childs[1];
        entry.data.func = fn;

        current_scope->table.push_back(entry);

        // Unimplemented();
        return InternDataType{DataTypeTag::None};
    }

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("cond")))
    {
        //  (cond ((< a b) a)
        //        (else  b))
        // Iterate over every child
        for (auto &branch : expr->childs)
        {
            // special case ... if the predicate is else, evaluate it true without evaluating it further
            // It could be made redundant though.
            if (lib::StrCmpEqual(branch->op.begin, branch->op.end, UNPACK_STR("else")))
                return EvaluateExpressionTree(branch->childs[0]);

            auto res = EvaluateExpressionTree(branch->childs[0]);
            if (res.tag == DataTypeTag::Int && res.data.integer)
            {
                return EvaluateExpressionTree(branch->childs[1]);
            }
        }
        Unimplemented();
    }

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("if")))
    {
        //  (cond ((< a b) a)
        //        (else  b))
        // Iterate over every child
        auto res = EvaluateExpressionTree(expr->childs[0]);

        Assert(expr->childs.size() == 3);

        if (res.tag == DataTypeTag::Int && res.data.integer)
        {
            return EvaluateExpressionTree(expr->childs[1]);
        }
        else
            return EvaluateExpressionTree(expr->childs[2]);

        Unimplemented();
    }

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("and")))
        return ApplyLogicalAnd(expr);
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("or")))
        return ApplyLogicalOr(expr);
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("not")))
        return ApplyLogicalNot(expr);
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("remainder")))
        return InternDataType{DataTypeTag::Int, EvaluateExpressionTree(expr->childs[0]).data.integer %
                                                    EvaluateExpressionTree(expr->childs[1]).data.integer};

    // This completes the let over lambda
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("let")))
        return DefineLet(expr);
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("lambda")))
        return DefineLambda(expr);

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("cons")))
        return DefineCons(expr);
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("car")))
        return Car(expr);
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("cdr")))
        return Cdr(expr);

    return HandleUserDefinedFunctions(expr);
    // Unimplemented();
}

InternDataType EvaluateExpressionTree(Expression *expr)
{
    if (expr->leaf)
        return EvaluateLeaf(expr);

    usize len = expr->op.begin - expr->op.begin;

    // All of these could be defined internally as library functions
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("+")))
        return AddExpressions(expr->childs);

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("-")))
        return SubExpressions(expr->childs);

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("*")))
        return MultiplyExpressions(expr->childs);

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("/")))
        return DivideExpressions(expr->childs);

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("<")))
        return ApplyComparisons(EvaluateExpressionTree(expr->childs[0]), EvaluateExpressionTree(expr->childs[1]),
                                [](auto x, auto y) { return x < y; });

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR(">")))
        return ApplyComparisons(EvaluateExpressionTree(expr->childs[0]), EvaluateExpressionTree(expr->childs[1]),
                                [](auto x, auto y) { return x > y; });

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("=")))
        return ApplyComparisons(EvaluateExpressionTree(expr->childs[0]), EvaluateExpressionTree(expr->childs[1]),
                                [](auto x, auto y) { return x == y; });

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("<=")))
        return ApplyComparisons(EvaluateExpressionTree(expr->childs[0]), EvaluateExpressionTree(expr->childs[1]),
                                [](auto x, auto y) {
                                    std::cout << typeid(x).name() << typeid(y).name();
                                    return x <= y;
                                });

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR(">=")))
        return ApplyComparisons(EvaluateExpressionTree(expr->childs[0]), EvaluateExpressionTree(expr->childs[1]),
                                [](auto x, auto y) { return x >= y; });

    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("random")))
    {
        InternDataType random = {DataTypeTag::Int};
        random.data.integer   = rand() % EvaluateExpressionTree(expr->childs[0]).data.integer;
        return random;
    }
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("sine")))
    {
        InternDataType random = {DataTypeTag::Real};
        random.data.real      = sin(EvaluateExpressionTree(expr->childs[0]).data.real);
        return random;
    }
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("cosine")))
    {
        InternDataType random = {DataTypeTag::Real};
        random.data.real      = cos(EvaluateExpressionTree(expr->childs[0]).data.real);
        return random;
    }
    if (lib::StrCmpEqual(expr->op.begin, expr->op.end, UNPACK_STR("tangent")))
    {
        InternDataType random = {DataTypeTag::Int};
        random.data.real      = tan(EvaluateExpressionTree(expr->childs[0]).data.real);
        return random;
    }
    // HandleCompareExpresions(expr);

    return HandleBuiltinFunctions(expr);
}
} // namespace Eval
