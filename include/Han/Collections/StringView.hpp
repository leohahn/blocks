#pragma once

#include "Han/Core.hpp"
#include <assert.h>
#include <string.h>

struct StringView
{
    const char* data;
    size_t len;

public:
    StringView()
        : data(nullptr)
        , len(0)
    {}

    StringView(const char* str)
        : data(str)
        , len(strlen(str))
    {
        assert(str);
    }

    StringView(const char* str, size_t len)
        : data(str)
        , len(len)
    {
        assert(str);
    }

    bool IsEmpty() const { return len == 0; }

    char Back() const { return data[len - 1]; }

    char Front() const { return data[0]; }

    char operator[](size_t index) const { return data[index]; }

    bool operator==(const char* other) const
    {
        return strncmp(data, other, HAN_MIN(len, strlen(other))) == 0;
    }
};
