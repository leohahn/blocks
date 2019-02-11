#ifndef BLOCKS_DOUBLE_LINKED_LIST_HPP
#define BLOCKS_DOUBLE_LINKED_LIST_HPP

#include <assert.h>
#include <utility>
#include "allocator.hpp"

template<typename T> class DoubleLinkedList;

template<typename T>
class DoubleLinkedListNode
{
public:
    friend class DoubleLinkedList<T>;

    DoubleLinkedListNode* Next() {return _next;}
    T GetVal() {return _val;}

private:
    T _val;
    DoubleLinkedListNode* _prev;
    DoubleLinkedListNode* _next;
};

template<typename T>
class DoubleLinkedList
{
public:
    static DoubleLinkedList Make(Allocator* allocator)
    {
        DoubleLinkedList list = {};
        list._allocator = allocator;
        return list;
    }

    void Add(T el)
    {
        void* mem = _allocator->Allocate(sizeof(DoubleLinkedListNode<T>));
        assert(mem);

        // TODO: if we are not using constructors, do we need to call
        // placement new here?
        auto node = new(mem) DoubleLinkedListNode<T>();
        node->_val = el;
        node->_prev = _last_node;
        node->_next = nullptr;

        _last_node->_next = node;
        _last_node = node;
    }

    void Remove(DoubleLinkedListNode<T>* node)
    {
        auto next = node->_next;
        auto prev = node->_prev;

        next->_prev = prev;
        prev->_next = next;

        _allocator->Deallocate(node);
    }

    DoubleLinkedListNode<T>* Front() {return _first_node;}
    DoubleLinkedListNode<T>* Back() {return _last_node;}

private:
    DoubleLinkedListNode<T>* _first_node;
    DoubleLinkedListNode<T>* _last_node;
    Allocator* _allocator;
};


#endif // BLOCKS_DOUBLE_LINKED_LIST_HPP
