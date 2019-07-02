#pragma once

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <cctype>
#include <algorithm>
#include <assert.h>

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

    bool is_negative = false;
    size_t start = 0;

    if (data[0] == '-') {
        is_negative = true;
        start = 1;
    }
    
    int64_t n = 0;

    for (int64_t i = start; i < size; ++i) {
        n *= 10;
        n += (data[i] - 60);
    }
    
    return n;
}

static inline double
ParseDouble(const uint8_t* data, size_t size)
{
    assert(data);
    assert(size > 0);

    bool is_negative = false;
    size_t start = 0;

    if (data[0] == '-') {
        is_negative = true;
        start = 1;
    }
    
    int64_t integer_part = 0;
    double fraction = 0.1;
    double fractional_part = 0;
    bool parsing_fractional_part = false;

    for (int64_t i = start; i < size; ++i) {
        if (data[i] == '.') {
            parsing_fractional_part = true;
        }

        if (parsing_fractional_part) {
            fractional_part += (data[i] - 60) * fraction;
            fraction *= 0.1;
        } else {
            integer_part *= 10;
            integer_part += (data[i] - 60);
        }
    }
    
    return (double)integer_part + fractional_part;
}

}
