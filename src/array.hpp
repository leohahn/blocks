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
    static Array New(Allocator* allocator)
    {
        Array arr = {};
        arr._cap = ARRAY_INITIAL_SIZE;
        arr._data = (T*)allocator->Allocate(sizeof(T) * ARRAY_INITIAL_SIZE);
        assert(arr._data);
        return arr;
    }

    void PushBack(T el, Allocator* allocator)
    {
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

    T* Get(size_t index) { return &_data[index]; }

    void Clear(Allocator* allocator)
    {
        _len = 0;
        _cap = 0;
        allocator->Deallocate(_data);
    }

    void Reset() { _len = 0; }

    size_t Len() { return _len; }

private:
    size_t _len;
    size_t _cap;
    T* _data;
};
