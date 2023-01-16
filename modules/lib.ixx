#include "../include/types.hpp"

#include <iostream>

export module Lib;
export namespace lib
{
struct DefaultHash
{
    u32 operator()(u32 x)
    {
        return x;
    }
    u32 operator()(f64 f)
    {
        return *reinterpret_cast<u32 *>(&f);
    }

    u32 operator()(const char *str)
    {
        u32 val = 0;
        while (*str)
            val = *str++;
        return val;
    }
};
//
// template <typename K, typename V> struct HashMap
//{
//    // use quadratic probing
//    // use array to implement probing
//    T  *data;
//    u32 capacity = 0;
//    u32 len      = 0;
//
//    // TODO :: Implement the hash table
//};

export bool StrCmpEqual(const u8 *begin, const u8 *end, const u8 *op, u32 len2)
{
    u32  len1 = end - begin + 1;
    bool eq   = true;

    eq        = len1 == len2;

    while (begin <= end && eq)
        eq = *begin++ == *op++;

    return eq;
}

// Sentinel based linked list doesn't work for recursive types
template <typename T> struct List
{
    struct Node
    {
        T     data;
        Node *next;
    };

    u32   count = 0;
    Node *head;

    List()
    {
        sentinel = new Node(); // Maybe  this is redundant too
        head     = sentinel;
    }

    const Node *GetSentinel() const
    {
        return sentinel;
    }

    Node *GetFirstNode()
    {
        return head;
    }

    void RemoveFirstNode()
    {
        auto back = head;
        head      = head->next;
        delete back;
    }

    void AddNode(T val)
    {
        Node *newnode = new Node{val, head};
        head          = newnode;
    }

    Node *SearchNode(T val)
    {
        sentinel->data = val;
        Node *ptr      = head;
        while (val != ptr->data)
            ptr = ptr->next;
        // return (ptr != &sentinel) * ptr;
        if (ptr != sentinel)
            return ptr;
        return nullptr;
    }

    bool RemoveNode(Node *node)
    {
        Node *prev = head, *ptr = head;

        while (ptr != node)
        {
            prev = ptr;
            ptr  = ptr->next;
        }

        if (ptr == head)
        {
            head = head->next;
            delete ptr;
            return true;
        }

        if (ptr != sentinel)
        {
            prev->next = ptr->next;
            delete ptr;
            return true;
        }
        return false;
    }

    bool RemoveNode(T val)
    {
        Node *prev = head, *ptr = head;
        while (val != ptr->data)
        {
            prev = ptr;
            ptr  = ptr->next;
        }

        if (ptr == head)
        {
            head = head->next;
            delete ptr;
            return true;
        }

        if (ptr != sentinel)
        {
            prev->next = ptr->next;
            delete ptr;
            return true;
        }
        return false;
    }

    void DestroyList()
    {
    }

    ~List()
    {
        for (auto ptr = head; ptr != sentinel;)
        {
            auto tmp = ptr->next;
            delete ptr;
            ptr = tmp;
        }
        delete sentinel;
    }

  private:
    Node *sentinel;
};

template <typename T>
constexpr std::ostream &operator<<(std::ostream &os, const List<T> &lst)
requires requires(T x) { os << x; }
{
    auto head = lst.head;

    while (head != lst.GetSentinel())
    {
        os << head->data << "  ";
        head = head->next;
    }
    return os << std::endl;
}
} // namespace lib
