#pragma once

#include "Allocator.hpp"
#include "Collections/StringView.hpp"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static constexpr const size_t kStringInitialCapacity = 8;

struct String
{
    Allocator* allocator;
    size_t cap;
    size_t len;
    char* data;

    String()
        : allocator(nullptr)
        , cap(0)
        , len(0)
        , data(nullptr)
    {}

    ~String() { assert(data == nullptr && "string should be freed"); }

    void Create(Allocator* allocator, const char* str = nullptr)
    {
        assert(len == 0);
        assert(cap == 0);
        assert(allocator);

        this->allocator = allocator;
        if (str) {
            size_t len = strlen(str);
            cap = len;
            data = (char*)allocator->Allocate(len + 1); // we need to count the null terminator
            memcpy(data, str, len);
            data[len] = 0; // null terminated string
        } else {
            cap = kStringInitialCapacity;
            data = (char*)allocator->Allocate(cap);
        }

        assert(data);
    }

    void Create(Allocator* allocator, const StringView& str)
    {
        assert(str.data && str.len > 0);
        assert(len == 0);
        assert(cap == 0);
        assert(allocator);

        this->allocator = allocator;
        this->cap = str.len;
        this->len = str.len;
        this->data = (char*)allocator->Allocate(len + 1); // we need to count the null terminator
        memcpy(data, str.data, str.len);
        data[len] = 0; // null terminated string

        assert(data);
    }

    void Destroy()
    {
        assert(data);
        assert(cap > 0);
        allocator->Deallocate(data);
        cap = 0;
        len = 0;
        data = nullptr;
        allocator = nullptr;
    }

    void Append(char character)
    {
        assert(data);
        size_t new_len = len + 1;
        while (new_len > (cap - 1)) {
            Resize();
        }
        data[len] = character;
        len = new_len;
    }

    void Resize()
    {
        // Need to allocate more memory
        size_t new_cap = cap * 1.5f;
        char* new_data = (char*)allocator->Allocate(new_cap);
        assert(new_data);
        memcpy(new_data, data, len);
        allocator->Deallocate(data);
        data = new_data;
        cap = new_cap;
    }

    void Append(const char* str) { Append(str, strlen(str)); }

    void Append(const StringView& str) { Append(str.data, str.len); }

    void Append(const StringView& str, size_t index_offset)
    {
        assert(index_offset < str.len);
        Append(str.data + index_offset, str.len - index_offset);
    }

    void Append(const char* str, size_t str_len)
    {
        assert(data);
        assert(str);

        size_t new_len = len + str_len;
        while (new_len > (cap - 1)) {
            Resize();
        }
        memcpy(data + len, str, str_len);
        len = new_len;
    }

    char Back() const { return data[len - 1]; }

    char Front() const { return data[0]; }

    char& operator[](size_t index) { return data[index]; }

    char operator[](size_t index) const { return data[index]; }

    StringView View() const { return StringView(data, len); }
};
