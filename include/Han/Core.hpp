#pragma once

#include "Han/Logger.hpp"
#include "Han/Allocator.hpp"

#define KILOBYTES(x) (x) * 1024
#define MEGABYTES(x) KILOBYTES(x) * 1024
#define GIGABYTES(x) MEGABYTES(x) * 1024

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef NDEBUG
#define HAN_PRODUCTION 1
#else
#define HAN_DEBUG 1
#endif

#define HAN_BIT(x) (1 << (x))
#define HAN_MIN(a, b) ((a) < (b) ? (a) : (b))
#define HAN_MIN3(a, b, c) MIN(MIN(a, b), c)
#define HAN_MAX(a, b) ((a) > (b) ? (a) : (b))
#define HAN_MAX3(a, b, c) MAX(MAX(a, b), c)
#define HAN_ABS(x) ((x) < 0) ? -(x) : (x)

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
#ifdef HAN_COMPILING_DLL
#define HAN_API __declspec(dllexport)
#else
#define HAN_API __declspec(dllimport)
#endif
#else
#define HAN_API
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

struct DeltaTime
{
public:
	DeltaTime() : DeltaTime(0.0f) {}
	DeltaTime(double t) : _delta_sec(t) {}

	operator double() const { return _delta_sec; }
	double InMilliseconds() const { return _delta_sec * 1000.0; }

private:
	double _delta_sec;
};

struct Time
{
public:
	Time() : Time(0.0) {}
	Time(double sec)
		: _time_seconds(sec)
	{}

	operator double() const { return _time_seconds; }

	DeltaTime operator-(Time other)
	{
		return DeltaTime(_time_seconds - other._time_seconds);
	}

private:
	double _time_seconds;
};
