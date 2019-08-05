#pragma once

#include "Logger.hpp"

#define KILOBYTES(x) (x) * 1024
#define MEGABYTES(x) KILOBYTES(x) * 1024
#define GIGABYTES(x) MEGABYTES(x) * 1024

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef NDEBUG
#define BLOCKS_PRODUCTION 1
#else
#define BLOCKS_DEBUG 1
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#else
#error "MIN is already defined"
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#else
#error "MIN is already defined"
#endif

#ifdef __GNUC__
#define COMPILER_GCC 1
#endif
#ifdef __clang__
#define COMPILER_CLANG 1
#endif
#ifdef _MSC_VER
#define COMPILER_MSC 1
#endif
#ifdef __linux__
#define OS_LINUX 1
#endif
#ifdef _WIN32
#define OS_WINDOWS 1
#endif
#ifdef __APPLE__
#define OS_APPLE 1
#endif
#if defined(unix) || defined(__unix__) || defined(__unix)
#define PLATFORM_UNIX 1
#endif

#if defined(__x86_64__) || defined(_M_X86) || defined(__i386__)
#define ARCH_X86 1
#endif

#define DISABLE_OBJECT_COPY(Type) \
    Type& operator=(const Type& t) = delete; \
    Type(const Type& t) = delete

#define DISABLE_OBJECT_MOVE(Type) \
    Type& operator=(Type&& t) = delete; \
    Type(Type&& t) = delete

#define DISABLE_OBJECT_COPY_AND_MOVE(Type) \
    DISABLE_OBJECT_COPY(Type); \
    DISABLE_OBJECT_MOVE(Type)

#define ASSERT(cond, msg) \
    if (!(cond)) { \
        LOG_ERROR("%s", "Assertion failed: " #cond); \
        LOG_ERROR("%s", "Message: " ## msg); \
        int* val = nullptr; \
        *val = 0xff;\
    }

#ifdef _DEBUG
struct ResourceManager;
extern ResourceManager* g_debug_resource_manager;
#endif
