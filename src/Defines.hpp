#pragma once

#include "Logger.hpp"
#include "Allocator.hpp"
#include "Application.hpp"

#define KILOBYTES(x) (x) * 1024
#define MEGABYTES(x) KILOBYTES(x) * 1024
#define GIGABYTES(x) MEGABYTES(x) * 1024

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef NDEBUG
#define BLOCKS_PRODUCTION 1
#else
#define BLOCKS_DEBUG 1
#endif

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#else
#error "MIN is already defined"
#endif

#ifndef MIN3
#define MIN3(a, b, c) MIN(MIN(a, b), c)
#else
#error "MIN3 is already defined"
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#else
#error "MIN is already defined"
#endif

#ifndef MAX3
#define MAX3(a, b, c) MAX(MAX(a, b), c)
#else
#error "MAX3 is already defined"
#endif

#ifndef ABS
#define ABS(x) ((x) < 0) ? -(x) : (x)
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

#if OS_WINDOWS
#ifdef BLOCKS_COMPILING_DLL
#define BLOCKS_API __declspec(dllexport)
#else
#define BLOCKS_API __declspec(dllimport)
#endif
#else
#define BLOCKS_API
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
        LOG_ERROR("Assertion failed: %s", #cond); \
        LOG_ERROR("Message %s", #msg); \
        fflush(stdout); \
        int* val = nullptr; \
        *val = 0xdeadbeef; \
    }

#define UNREACHABLE ASSERT(false, "Unreachable code")

#ifdef _DEBUG
struct ResourceManager;
extern ResourceManager* g_debug_resource_manager;
#endif

class EngineInterface;

typedef Application* (*AppFactoryFunction)(Allocator*, EngineInterface*);

struct InitData
{
    AppFactoryFunction app_factory;
};

typedef void(*InitializePluginFunction)(InitData* init_data);

