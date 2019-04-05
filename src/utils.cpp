#include "Utils.hpp"
#include "Defines.hpp"
#include <assert.h>
#include <stdio.h>

static const char* kSizeNames[] = {
    "B", "KB", "MB", "GB", "TB",
};

thread_local static char g_buf[64];

char*
Utils::GetPrettySize(size_t size)
{
    unsigned index = 0;
    size_t new_size = size;
    while (new_size >= 1024) {
        new_size /= 1024;
        ++index;
    }
    assert(index < ARRAY_SIZE(kSizeNames));
    snprintf(g_buf, 64, "%lu %s", new_size, kSizeNames[index]);
    return g_buf;
}



