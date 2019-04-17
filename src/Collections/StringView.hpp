#pragma once

#include <assert.h>
#include <string.h>

struct StringView
{
    const char* data;
    size_t len;

public:
    StringView(const char* str)
        : data(str)
        , len(strlen(str))
    {
        assert(str);
        assert(len > 0);
    }

    StringView(const char* str, size_t len)
        : data(str)
        , len(len)
    {
        assert(str);
        assert(len > 0);
    }

    char Back() const { return data[len - 1]; }

    char Front() const { return data[0]; }

    char operator[](size_t index) const { return data[index]; }
};
