#include "../include/types.hpp"
#include "../include/macros.hpp"

// STL headers as modules
import <vector>;
import <ranges>;
namespace ranges = std::ranges;

import Lib;
import Log;

// Module partition for main module Eval
export module Eval:base;
namespace Eval
{

export enum class DataTypeTag
{
    Cons = 0,
    Int,
    Real,
    Ptr,
    Id, // Or id that needs to be resolved or errored
    Lambda,
    None
};

struct InternDataType;

template <typename T>
concept NativeType = std::is_same_v<T, i64> || std::is_same_v<T, f64> || std::is_same_v<T, TwoBytePtrs> ||
                     std::is_same_v<T, lib::List<InternDataType>> || std::is_same_v<T, TwoBytePtrs>;

struct Macros
{
};

struct Expression;

struct Funcs
{
    static constexpr u32 MAX_ARGS       = 10;
    u32                  args_count     = 0;

    TwoBytePtrs          args[MAX_ARGS] = {};

    Expression          *body; // ?? Is it the right way of doing this? It may be or may be not though.
};

struct SymbolTable;

struct Lambda
{
    // Lambda are anonymous functions nothing fancy
    // Clone whole the symbol table, its the easiest solution, but the least efficient one
    // (lambda(x)(* x y)) -> y is to be evaluated from the surrounding scope
    // One solution is to traverse the function body and replace the symbols not defined as lambda parameters,its not
    // straightforward to implement it though
    Funcs        func;
    TwoBytePtrs  name;
    SymbolTable *closure;
};

struct Cons;

export struct InternDataType
{
    DataTypeTag tag; // Tag isn't required if variants are used

    // std::variant<std::monostate, std::vector<InternDataType>, i64, f64, usize> data; // use visitor patterns for this
    union
    {
        i64         integer;
        f64         real = 0.0f;

        TwoBytePtrs id;

        usize       ptr;

        Cons       *cons;
        Lambda      lambda;
    } data;

    template <typename U>
    requires NativeType<U>
    U &get_value()
    {
        return *(U *)&data;
    }
};

struct Cons
{
    InternDataType car;
    InternDataType cdr;
};

struct SymbolTableEntry
{
    enum class EntryType
    {
        Var = 0, // Variable with a value
        Id,      // Unresolved identifier
        Macro,
        Function,
        Lambda,
        None
    };

    SymbolTableEntry()     = default;

    EntryType   entry_type = EntryType::None;
    TwoBytePtrs name;

    union
    {
        InternDataType var = {DataTypeTag::None};
        Macros         macro;
        Funcs          func;
    } data;
};

export struct Expression
{
    Expression()                   = default;

    bool                      leaf = false;
    TwoBytePtrs               op; // if there are multiple types apply it recursively
    InternDataType            val;
    std::vector<Expression *> childs;
};

struct SymbolTable
{
    std::vector<SymbolTableEntry> table;

    void                          dump()
    {
        constexpr const char *types[] = {"Var", "Id", "Macro", "Function", "Lambda", "None"};

        for (auto const &x : table)
        {
            GetSingletonLogger().Log(
                types[u32(x.entry_type)], " :  ",
                std::string_view((const char *)x.name.begin, static_cast<size_t>(x.name.end - x.name.begin + 1)));
        }
    }

    SymbolTableEntry *GetEntryInTable(InternDataType &type)
    {
        auto it = ranges::find_if(table, [&](auto const &elem) {
            if (true) //(elem.entry_type == SymbolTableEntry::EntryType::Var)
            {
                if (lib::StrCmpEqual(type.data.id.begin, type.data.id.end, elem.name.begin,
                                     elem.name.end - elem.name.begin + 1))
                    return true;
            }
            return false;
        });
        if (it != table.end())
            return &(*it);
        return nullptr;
    }
};

export struct ScopeStack
{
    lib::List<SymbolTable> scope_stack;

    ScopeStack()
    {
        scope_stack.AddNode(SymbolTable()); // This will serve as global scope for  the interpreter
    }

    SymbolTable *GetStackTop()
    {
        return &scope_stack.GetFirstNode()->data;
    }

    SymbolTable *PushStack(const SymbolTable &table)
    {
        scope_stack.AddNode(table);
        return &scope_stack.GetFirstNode()->data;
    }

    SymbolTableEntry *GetSymbolEntry(InternDataType &key)
    {
        for (auto begin = scope_stack.head; begin != scope_stack.GetSentinel(); begin = begin->next)
        {
            if (auto x = begin->data.GetEntryInTable(key); x)
                return x;
        }
        return nullptr;
    }
    void PopStack()
    {
        scope_stack.RemoveFirstNode();
    }
};
} // namespace Eval
// A pretty simple garbage collector
// Only list and lambda's closure are allocated via new, so only list will be used to allocate and free the memory
struct AllocData
{
    bool   is_garbage = false;
    bool   is_array   = false;
    void  *ptr;
    size_t size; // size of the allocated data
};

// We would like to have

export struct GarbageCollector
{
    // A simple vector
    std::vector<AllocData> allocations;

    void                   Run(Eval::ScopeStack *bounding_scope);
    void                   AddAllocation(void *ptr, size_t size);
    bool                   MarkNotGarbage(void *ptr);
    void                   FreeGarbage();

  private:
    void DetectGarbageFromSymbolTable(Eval::SymbolTable &table);
};
