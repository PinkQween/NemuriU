#ifndef CVN_LOG_H
#define CVN_LOG_H

/* Cross-platform logging for CVN */

#ifdef __WIIU__
/* For Wii U, use OSReport which shows in Cemu logs */
#include <coreinit/debug.h>

#define CVN_LOG_INIT() ((void)0)
#define CVN_LOG(fmt, ...) OSReport("[CVN] " fmt "\n", ##__VA_ARGS__)
#define CVN_LOG_CONSOLE_DRAW() ((void)0)
#define CVN_LOG_SHUTDOWN() ((void)0)

#else
#include <stdio.h>

#define CVN_LOG_INIT() ((void)0)
#define CVN_LOG(fmt, ...) printf("[CVN] " fmt "\n", ##__VA_ARGS__)
#define CVN_LOG_CONSOLE_DRAW() ((void)0)
#define CVN_LOG_SHUTDOWN() ((void)0)

#endif

#endif /* CVN_LOG_H */
