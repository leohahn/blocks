#pragma once

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <cctype>
#include <algorithm>
#include <assert.h>
#include "Han/Collections/StringView.hpp"
#include "Han/Collections/String.hpp"

#define FIRST_ASCII_NUMBER 48

namespace Utils
{

String GetPrettySize(size_t size, Allocator* alloc = MallocAllocator::Instance());

int ParseInt32(const char* str, int32_t* res);
int ParseInt32(const uint8_t* data, size_t size, int32_t* res);

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
    int64_t start = 0;

    if (data[0] == '-') {
        is_negative = true;
        start = 1;
    }
    
    int64_t n = 0;

    for (int64_t i = start; i < (int64_t)size; ++i) {
        n *= 10;
        n += (int64_t)(data[i] - FIRST_ASCII_NUMBER);
    }
    
    return n;
}

static inline double
ParseDouble(const uint8_t* data, size_t size)
{
    assert(data);
    assert(size > 0);

    bool is_negative = false;
    int64_t start = 0;

    if (data[0] == '-') {
        is_negative = true;
        start = 1;
    }
    
    int64_t integer_part = 0;
    int64_t fractional_part = 0;
    bool parsing_fractional_part = false;
    int64_t num_zeroes_fractional_part = 0;

    //
    // TODO, FIXME: there is probably a more efficient way of implementing this function.
    //

	// The exponent part of the number
	int32_t exp = 0;

    for (int64_t i = start; i < (int64_t)size; ++i) {
		if (data[i] == 'e') {
			// Parse exponential number
			++i;
			int consumed;
			if (!(consumed = ParseInt32(data + i, size - i, &exp))) {
				ASSERT(false, "should not fail to parse exponential");
				return false;
			}
			i += consumed;
			ASSERT(i >= size, "Scientific notation should be the last part of the number");
			break;
		}
        if (data[i] == '.') {
            ++i;
            parsing_fractional_part = true;
        }

        uint8_t curr_number = data[i] - FIRST_ASCII_NUMBER;

        if (parsing_fractional_part) {
            fractional_part *= 10;
            fractional_part += curr_number;
			++num_zeroes_fractional_part;
        } else {
            integer_part *= 10;
            integer_part += curr_number;
        }
    }

	double real_fractional_part = fractional_part;
    for (int i = 0; i < num_zeroes_fractional_part; ++i) {
		real_fractional_part /= 10;
    }

    double parsed = (double)integer_part + real_fractional_part;
	parsed = parsed * pow(10, exp);

    return is_negative ? -parsed : parsed;
}

inline String
ToString(int i)
{
	char str[100];
    sprintf(str, "%d", i);
    return String(str);
}

inline String
ToString(float f)
{
	char str[100];
	sprintf(str, "%.2f", f);
	return String(str);
}

}

namespace StringUtils
{
    bool EndsWith(const char* str, const char* ending);
    bool FindFromRight(const StringView& str, char c, size_t* out_index);
    StringView Trim(const StringView& str);
}
