#pragma once

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <cctype>

namespace Utils
{

char* GetPrettySize(size_t size);

bool ParseInt32(const char* str, int32_t* res);

bool StringEndsWith(const char* str, const char* ending);

static inline uint8_t*
EatUntil(char c, uint8_t* it, const uint8_t* end_it)
{
    uint8_t* new_it = it;
    while (*new_it != c && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

template<typename T>
static inline uint8_t *
EatUntil(const T &chars, uint8_t *it, const uint8_t *end_it)
{
    uint8_t *new_it = it;
    while (std::find(std::begin(chars), std::end(chars), *new_it) == std::end(chars) &&
        new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

static inline uint8_t *
EatWhile(const std::function<bool(uint8_t)> &predicate, uint8_t *it, const uint8_t *end_it)
{
    uint8_t *new_it = it;
    while (predicate(*new_it) && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

static inline uint8_t *
EatWhitespaces(uint8_t* it, const uint8_t* end_it)
{
    uint8_t* new_it = it;
    while (std::isspace(*new_it) && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}


}
