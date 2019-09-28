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

bool
Utils::ParseInt32(const char* str, int32_t* res)
{
    assert(res);
    const char* search = str;
    *res = 0;
    while (search) {
        if (isdigit(*search)) {
            *res = (*res * 10) + (*search - kStartAsciiTableOffset);
        } else {
            return false;
        }
    }

    return true;
}

bool
StringUtils::EndsWith(const char* str, const char* ending)
{
    assert(str);
    assert(ending);

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