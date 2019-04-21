#pragma once

#include "Allocator.hpp"
#include "Collections/StringView.hpp"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static constexpr const size_t kStringInitialCapacity = 8;
static constexpr const float kStringGrowthFactor = 1.5f;

struct String
{
    Allocator* allocator;
    size_t cap;
    size_t len;
    char* data;

    String()
        : String(nullptr)
    {}

    String(Allocator* allocator)
        : allocator(allocator)
        , cap(0)
        , len(0)
        , data(nullptr)
    {}

    String(const String& str)
        : String(nullptr)
    {
        assert(str.allocator);
        *this = str;
    }

    String(String&& str)
        : String()
    {
        *this = std::move(str);
    }

    // Copy assignment
    String& operator=(const String& str)
    {
        assert(this != &str);
        assert(str.allocator);
        ShallowCopyFields(str);
        // copy the contents as well
        if (data) {
            allocator->Deallocate(data);
        }
        data = (char*)allocator->Allocate(str.len + 1);
        assert(data);
        memcpy(data, str.data, str.len);
        data[len] = '\0';
        return *this;
    }

    // Move assignment
    String& operator=(String&& str)
    {
        assert(this != &str);
        Destroy();
        ShallowCopyFields(str);
        str.allocator = nullptr;
        str.data = nullptr;
        str.len = 0;
        str.cap = 0;
        return *this;
    }

    ~String() { assert(data == nullptr && "string should be freed"); }

    void Destroy()
    {
        if (data) {
            allocator->Deallocate(data);
        }
        cap = 0;
        len = 0;
        data = nullptr;
        allocator = nullptr;
    }

    void Resize()
    {
        // Need to allocate more memory
        size_t new_cap = cap * kStringGrowthFactor;
        char* new_data = (char*)allocator->Allocate(new_cap);
        assert(new_data);
        memcpy(new_data, data, len);
        new_data[len] = 0;
        allocator->Deallocate(data);
        data = new_data;
        cap = new_cap;
    }

    void Append(char character)
    {
        assert(data);
        size_t new_len = len + 1;
        while (new_len > (cap - 1)) {
            Resize();
        }
        assert(cap > len);
        data[len] = character;
        data[len+1] = 0;
        len = new_len;
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
        assert(str);
        if (data) {
            size_t new_len = len + str_len;
            while (new_len > (cap - 1)) {
                Resize();
            }
            memcpy(data + len, str, str_len);
            len = new_len;
        } else {
            data = (char*)allocator->Allocate(str_len + 1);
            assert(data);
            memcpy(data, str, str_len);
            data[str_len] = 0;
            len = str_len;
            cap = str_len;
        }
        data[len] = 0; // add null terminator
    }

    char Back() const { return data[len - 1]; }

    char Front() const { return data[0]; }

    char& operator[](size_t index) { return data[index]; }

    char operator[](size_t index) const { return data[index]; }

    bool operator==(const String& str)
    {
        if (len != str.len) {
            return false;
        }
        return memcmp(data, str.data, len) == 0;
    }
    
    bool IsEmpty() const { return len == 0; }

    StringView View() const { return StringView(data, len); }

private:
    void ShallowCopyFields(const String& str)
    {
        allocator = str.allocator;
        cap = str.cap;
        len = str.len;
        data = str.data;
    }
};

inline bool
operator==(const String& lhs, const StringView& rhs)
{
    if (lhs.len != rhs.len) {
        return false;
    }
    return memcmp(lhs.data, rhs.data, rhs.len) == 0;
}

inline bool
operator==(const StringView& lhs, const String& rhs)
{
    return rhs == lhs;
}