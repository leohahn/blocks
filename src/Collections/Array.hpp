#pragma once

#include <stdlib.h>
#include <assert.h>
#include "Allocator.hpp"

#define ARRAY_INITIAL_SIZE 2
#define ARRAY_INVALID_POS ((size_t)-1)

template<typename T>
struct Array
{
    Array()
        : Array(nullptr)
    {}
    
    Array(Allocator* allocator)
        : allocator(allocator)
        , len(0)
        , cap(0)
        , data(nullptr)
    {}

    // Copy semantics
    Array(const Array& arr)
        : allocator(nullptr)
        , len(0)
        , cap(0)
        , data(nullptr)
    {
        *this = arr;
    }

    Array& operator=(const Array& arr)
    {
        if (data) {
            allocator->Deallocate(data);
        }
        allocator = arr.allocator;
        len = arr.len;
        cap = arr.cap;
        data = (T*)allocator->Allocate(arr.cap * sizeof(T));

        assert(data && "copy should not fail");
        memcpy(data, arr.data, sizeof(T) * arr.len);
        return *this;
    }

    // Move semantics
    Array(Array&& arr)
        : allocator(nullptr)
        , len(0)
        , cap(0)
        , data(nullptr)
    {
        *this = std::move(arr);
    }

    Array& operator=(Array&& arr)
    {
        if (data) {
            allocator->Deallocate(data);
        }
        allocator = arr.allocator;
        data = arr.data;
        len = arr.len;
        cap = arr.cap;
        arr.allocator = nullptr;
        arr.data = nullptr;
        arr.len = 0;
        arr.cap = 0;
        return *this;
    }

    ~Array()
    {
        assert(data == nullptr && "array should be destroyed");
    }

    void Destroy()
    {
        if (data) {
            allocator->Deallocate(data);
            data = nullptr;
        }
        allocator = nullptr;
        len = 0;
        cap = 0;
    }

    void PushBack(T el)
    {
        if (!data) {
            cap = ARRAY_INITIAL_SIZE;
            data = (T*)allocator->Allocate(sizeof(T) * ARRAY_INITIAL_SIZE);
            len = 0;
            assert(data);
        }

        if (len == cap) {
            // Resize array
            size_t new_cap = (size_t)(cap * 1.5f);
            size_t size = new_cap * sizeof(T);
            T* new_data = (T*)allocator->Allocate(size);
            assert(new_data);
            memcpy(new_data, data, len * sizeof(T));
            allocator->Deallocate(data);
            data = new_data;
            cap = new_cap;
        }

        new (data + len) T(std::move(el)); // create default instance of T
        ++len;
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

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    void Reset() { len = 0; }

    size_t GetLen() const { return len; }
    T* GetData() const { return data; }

    Allocator* allocator;
    size_t len;
    size_t cap;
    T* data;
};
