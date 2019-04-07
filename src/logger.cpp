#include "Logger.hpp"
#include <time.h>

static LogLevel g_log_level = LogLevel_Debug;

LogLevel
GetLogLevel()
{
    return g_log_level;
}

void
SetLogLevel(LogLevel level)
{
    g_log_level = level;
}

const char*
GetLogTag(LogLevel level)
{
    static constexpr const char* tags[] = {
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
        nullptr,
    };
    return tags[level];
}

const char *
PrettyTime()
{
    static char buffer[64];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);

    return buffer;
}
