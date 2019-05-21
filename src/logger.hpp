#pragma once

#include <string.h>
#include <stdio.h>
#include "Defines.hpp"

enum LogLevel
{
    LogLevel_Debug = 0,
    LogLevel_Info,
    LogLevel_Warn,
    LogLevel_Error,
    LogLevel_None,
};

LogLevel GetLogLevel();
void SetLogLevel(LogLevel level);
const char* GetLogTag(LogLevel level);
const char* PrettyTime();
void WindowsDebugPrint(const char* func, int line, const char* fmt, ...);

#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define LOG_FMT             "%s | %-7s | %-15s | %s:%d | "
#define LOG_ARGS(LOG_TAG)   timenow(), LOG_TAG, _FILE, __FUNCTION__, __LINE__

#if 0
#define _LOG_PRINT_FUNCTION(fmt, ...) WindowsDebugPrint(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#else
#define _LOG_PRINT_FUNCTION(fmt, ...) fprintf(stderr, "%s | %-7s | %-15s || " fmt "\n", ## __VA_ARGS__)
#endif
#define _LOG_PRINT_FUNCTION_WRAPPER(level, fmt, ...) _LOG_PRINT_FUNCTION(fmt, PrettyTime(), GetLogTag(level), _FILE, ## __VA_ARGS__)

#if BLOCKS_DEBUG
#define LOG_DEBUG(fmt, ...) if (GetLogLevel() == LogLevel_Debug) { _LOG_PRINT_FUNCTION_WRAPPER(LogLevel_Debug, fmt, ## __VA_ARGS__); }
#else
#define LOG_DEBUG(fmt, ...)
#endif

#if BLOCKS_DEBUG
#define LOG_INFO(fmt, ...) if (GetLogLevel() <= LogLevel_Info) { _LOG_PRINT_FUNCTION_WRAPPER(LogLevel_Info, fmt, ## __VA_ARGS__); }
#else
#define LOG_INFO(fmt, ...)
#endif

#if BLOCKS_DEBUG
#define LOG_WARN(fmt, ...) if (GetLogLevel() <= LogLevel_Warn) { _LOG_PRINT_FUNCTION_WRAPPER(LogLevel_Warn, fmt, ## __VA_ARGS__); }
#else
#define LOG_WARN(fmt, ...)
#endif

#if BLOCKS_DEBUG
#define LOG_ERROR(fmt, ...) if (GetLogLevel() <= LogLevel_Error) { _LOG_PRINT_FUNCTION_WRAPPER(LogLevel_Error, fmt, ## __VA_ARGS__); }
#else
#define LOG_ERROR(fmt, ...)
#endif
