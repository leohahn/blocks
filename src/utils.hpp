#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Utils
{

char* GetPrettySize(size_t size);

bool ParseInt32(const char* str, int32_t* res);

bool StringEndsWith(const char* str, const char* ending);

}
