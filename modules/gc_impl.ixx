//
export module Eval:gc_impl;
import :base;

void GarbageCollector::AddAllocation(void *ptr, size_t size)
{
    allocations.emplace_back(AllocData{.ptr = ptr, .size = size});
}

void GarbageCollector::Run(Eval::ScopeStack *bounding_scope)
{
    // start from the current symbol table and look for all the local variables and function parameters
    // if its closure, look for their arguments
    // First mark all memory region as garbage
    for (auto &alloc : allocations)
        alloc.is_garbage = true;

    // Transitive links aren't follwed
    for (auto begin = bounding_scope->scope_stack.head; begin != bounding_scope->scope_stack.GetSentinel();
         begin      = begin->next)
    {
        // for each symbol table, find the symbol entry and determine whether it is allocating types or not
        DetectGarbageFromSymbolTable(begin->data);
    }
}

bool GarbageCollector::MarkNotGarbage(void *ptr)
{
    for (auto &entry : allocations)
    {
        if (entry.ptr == ptr)
        {
            entry.is_garbage = false;
            return true;
        }
    }
    return false;
}

void GarbageCollector::FreeGarbage()
{
    for (auto &entry : allocations)
    {
        if (entry.is_garbage)
        {
            delete entry.ptr;
        }
    }
}

void GarbageCollector::DetectGarbageFromSymbolTable(Eval::SymbolTable &scope)
{
    for (auto &entry : scope.table)
    {
        if (entry.entry_type == Eval::SymbolTableEntry::EntryType::Var)
        {
            // It can be a cons or lambda both of which allocates
            auto &var = entry.data.var; // which is of type InternDataType
            if (var.tag == Eval::DataTypeTag::Cons)
            {
                // It has probably allocated somewhere or it has been assigned to pointer that allocated it
                MarkNotGarbage(var.data.cons);
            }
            else if (var.tag == Eval::DataTypeTag::Lambda)
            {
                // Its closure has been definitely allocated, but make sure to check its closures's symbol table
                // stack entries
                MarkNotGarbage(var.data.lambda.closure);
                // Recurse this
                DetectGarbageFromSymbolTable(*var.data.lambda.closure);
            }
        }
    }
}