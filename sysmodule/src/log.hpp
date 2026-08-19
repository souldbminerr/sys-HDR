#pragma once

#ifdef DEBUG

void logInit();
void logClose();
void logPrintf(const char *fmt, ...);

#define LOG(...) logPrintf(__VA_ARGS__)

#else

#define LOG(...) ((void)0)

static inline void logInit() {}
static inline void logClose() {}

#endif
