#include "Han/Utils.hpp"
#include "Han/Core.hpp"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static const char* kSizeNames[] = {
    "B", "KiB", "MiB", "GiB", "TiB", "PiB"
};

String
Utils::GetPrettySize(size_t size, Allocator* alloc)
{
    uint32_t index = 0;
    double new_size = (double)size;
    while (new_size >= 1024) {
        new_size /= 1024;
        ++index;
    }
    ASSERT(index < ARRAY_SIZE(kSizeNames), "Index should be smaller than the array");

	char buf[64];
    snprintf(buf, 64, "%.2f %s", new_size, kSizeNames[index]);

	String str(alloc, buf);
    return str;
}

static constexpr int kStartAsciiTableOffset = 48;

int
Utils::ParseInt32(const char* str, int32_t* res)
{
	return ParseInt32((const uint8_t*)str, strlen(str), res);
}


int
Utils::ParseInt32(const uint8_t* data, size_t size, int32_t* res)
{
    ASSERT(res, "res should not be null");
    const uint8_t* search = data;
	const uint8_t* end = search + size;
    *res = 0;

	bool is_negative = false;
	int num_consumed = 0;

	ASSERT(search < end, "Search should always be smaller than end");
    while (search < end) {
		if (*search == '-') {
			is_negative = true;
			++search;
			++num_consumed;
		} else if (isdigit(*search)) {
            *res = (*res * 10) + (*search - kStartAsciiTableOffset);
			++search;
			++num_consumed;
        } else {
            return 0;
        }
    }

	if (is_negative) *res = -*res;
    return num_consumed;
}

bool
StringUtils::EndsWith(const char* str, const char* ending)
{
    ASSERT(str, "str should not be null");
    ASSERT(ending, "ending should not be null");

    const size_t str_len = strlen(str);
    const size_t ending_len = strlen(ending);

    if (ending_len > str_len) {
        return true;
    }

    for (int32_t i = ending_len - 1, j = (int32_t)str_len - 1; i >= 0; --j, --i) {
        if (str[j] != ending[i]) {
            return false;
        }
    }

    return true;
}

bool
StringUtils::FindFromRight(const StringView& str, char c, size_t* out_index)
{
    assert(out_index);

    for (size_t i = str.len - 1; i >= 0; --i) {
        if (str.data[i] == c) {
            *out_index = i;
            return true;
        }

        if (i == 0) {
            break;
        }
    }

    return false;
}

StringView
StringUtils::Trim(const StringView& str)
{
    const char* start = str.data;
    size_t len = str.len;

    while (len > 0 && std::isspace(*start)) {
        ++start;
        --len;
    }

    const char* end = start + len;

    while (len > 0 && std::isspace(*end)) {
        --end;
        --len;
    }

    if (len == 0) {
        return StringView();
    } else {
        return StringView(start, len);
    }
}