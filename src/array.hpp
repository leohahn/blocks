#pragma once

#include <stdlib.h>
#include <assert.h>
#include "Allocator.hpp"

#define ARRAY_INITIAL_SIZE 8
#define ARRAY_INVALID_POS ((size_t)-1)

template<typename T>
class Array
{
public:
    Array()
        : _len(0)
        , _cap(ARRAY_INITIAL_SIZE)
        , _data(nullptr)
    {}

    ~Array()
    {
        assert(_data == nullptr && "array should be destroyed");
    }

    void Create(Allocator* allocator)
    {
        _allocator = allocator;
        _cap = ARRAY_INITIAL_SIZE;
        _data = (T*)allocator->Allocate(sizeof(T) * ARRAY_INITIAL_SIZE);
        _len = 0;
        assert(_data);
    }

    void Destroy()
    {
        _allocator->Deallocate(_data);
        _data = nullptr;
        _allocator = nullptr;
        _len = 0;
        _cap = 0;
    }

    void PushBack(T el, Allocator* allocator)
    {
        assert(_data != nullptr && "the create function should be called");
        if (_len == _cap) {
            // Resize array
            size_t new_cap = _cap * 1.5f;
            size_t size = new_cap * sizeof(T);
            T* new_data = (T*)allocator->Allocate(size);
            assert(new_data);
            memcpy(new_data, _data, size);
            allocator->Deallocate(_data);
            _data = new_data;
        }

        _data[_len++] = el;
    }

    int IndexOf(T el)
    {
        for (size_t i = 0; i < _len; ++i) {
            if (el == _data[i]) {
                return i;
            }
        }

        return -1;
    }

    T* operator[](size_t index) { return &_data[index]; }

    void Reset() { _len = 0; }

    size_t len() { return _len; }
    T* data() { return _data; }

private:
    Allocator* _allocator;
    size_t _len;
    size_t _cap;
    T* _data;
};
