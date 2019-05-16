#include "Utils.hpp"
#include "Defines.hpp"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static const char* kSizeNames[] = {
    "B", "KiB", "MiB", "GiB", "TiB",
};

thread_local static char g_buf[64];

char*
Utils::GetPrettySize(size_t size)
{
    uint32_t index = 0;
    size_t new_size = size;
    while (new_size >= 1024) {
        new_size /= 1024;
        ++index;
    }
    assert(index < ARRAY_SIZE(kSizeNames));
    snprintf(g_buf, 64, "%lu %s", new_size, kSizeNames[index]);
    return g_buf;
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


