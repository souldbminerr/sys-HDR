#include "log.hpp"

#ifdef DEBUG

#include <cstdarg>
#include <cstdio>

#include <switch.h>

namespace {
FILE *s_logFile = nullptr;
}

void logInit()
{
    s_logFile = fopen("sdmc:/config/" CONFIG_DIR "/sys-hdr.log", "a");
    logPrintf("---- log start ----");
}

void logClose()
{
    if (!s_logFile)
        return;
    fclose(s_logFile);
    s_logFile = nullptr;
}

void logPrintf(const char *fmt, ...)
{
    if (!s_logFile)
        return;

    fprintf(s_logFile, "[%10llu] ", static_cast<unsigned long long>(armGetSystemTick()));

    va_list args;
    va_start(args, fmt);
    vfprintf(s_logFile, fmt, args);
    va_end(args);

    fputc('\n', s_logFile);
    fflush(s_logFile);
}

#endif
