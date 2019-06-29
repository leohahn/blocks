#pragma once

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <cctype>
#include <algorithm>

namespace Utils
{

char* GetPrettySize(size_t size);

bool ParseInt32(const char* str, int32_t* res);

bool StringEndsWith(const char* str, const char* ending);

static inline const uint8_t*
EatUntil(char c, const uint8_t* it, const uint8_t* end_it)
{
    const uint8_t* new_it = it;
    while (*new_it != c && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

template<typename T>
static inline const uint8_t *
EatUntil(const T &chars, const uint8_t* it, const uint8_t* end_it)
{
    const uint8_t *new_it = it;
    while (std::find(std::begin(chars), std::end(chars), *new_it) == std::end(chars) &&
        new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

static inline const uint8_t *
EatWhile(const std::function<bool(uint8_t)> &predicate, const uint8_t* it, const uint8_t* end_it)
{
    const uint8_t *new_it = it;
    while (predicate(*new_it) && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

static inline const uint8_t *
EatWhitespaces(const uint8_t* it, const uint8_t* end_it)
{
    const uint8_t* new_it = it;
    while (std::isspace(*new_it) && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}
    
static inline int64_t
ParseInt64(const uint8_t* data, size_t size)
{
    assert(data);
    assert(size > 0);
    
    int64_t n = 0;

    for (int64_t i = 0; i < size; ++i) {
        n *= 10;
        n += data[i];
    }
    
    return n;
}

}
