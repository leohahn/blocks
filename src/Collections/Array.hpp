#pragma once

#include <stdlib.h>
#include <assert.h>
#include "Allocator.hpp"

#define ARRAY_INITIAL_SIZE 8
#define ARRAY_INVALID_POS ((size_t)-1)

template<typename T>
struct Array
{
    Array()
        : len(0)
        , cap(ARRAY_INITIAL_SIZE)
        , data(nullptr)
    {}

    ~Array()
    {
        assert(data == nullptr && "array should be destroyed");
    }

    void Create(Allocator* allocator)
    {
        this->allocator = allocator;
        cap = ARRAY_INITIAL_SIZE;
        data = (T*)allocator->Allocate(sizeof(T) * ARRAY_INITIAL_SIZE);
        len = 0;
        assert(data);
    }

    void Destroy()
    {
        allocator->Deallocate(data);
        data = nullptr;
        allocator = nullptr;
        len = 0;
        cap = 0;
    }

    void PushBack(T el, Allocator* allocator)
    {
        assert(data != nullptr && "the create function should be called");
        if (len == cap) {
            // Resize array
            size_t new_cap = cap * 1.5f;
            size_t size = new_cap * sizeof(T);
            T* new_data = (T*)allocator->Allocate(size);
            assert(new_data);
            memcpy(new_data, data, size);
            allocator->Deallocate(data);
            data = new_data;
        }

        data[len++] = el;
    }

    int IndexOf(T el)
    {
        for (size_t i = 0; i < len; ++i) {
            if (el == data[i]) {
                return i;
            }
        }

        return -1;
    }

    T* operator[](size_t index) { return &data[index]; }

    void Reset() { len = 0; }

    size_t GetLen() const { return len; }
    T* GetData() const { return data; }

    Allocator* allocator;
    size_t len;
    size_t cap;
    T* data;
};
