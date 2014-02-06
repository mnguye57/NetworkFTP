/**
 *  A macro-based, variable-level debug printing functions
 *
 *  There are 5 levels available ranging from -1 (ERR) to 3 (VDBG)
 */

#ifndef __DBG_PRINT_H
#define __DBG_PRINT_H

// ============================================================================
#include <sys/types.h>
#include <unistd.h>
// ============================================================================
#define DBG_LEVEL_ERROR  -1
#define DBG_LEVEL_WARN    0
#define DBG_LEVEL_INFO    1
#define DBG_LEVEL_DEBUG   2
#define DBG_LEVEL_VDEBUG  3
// ============================================================================
#define PRINT_VERBOSE(LVL, FMT, ...) \
    dbg_print(LVL, "  (%u)(%-20s(%4u)::%-12s - " FMT, \
            getpid(), __FILE__, __LINE__, __FUNCTION__ , ##__VA_ARGS__)

#define PRINT_SIMPLE(LVL, FMT, ...) \
    dbg_print(LVL, "  (%u)%-12s - " FMT, \
            getpid(), __FUNCTION__ , ##__VA_ARGS__)

#define PRINT_TEST(LVL, FMT , ...) \
    fprintf(stderr, "%-12s - " FMT, \
            __FUNCTION__ , ##__VA_ARGS__)
// ============================================================================
#define ERR_PRINT(FMT, ...)      PRINT_VERBOSE(DBG_LEVEL_ERROR, "ERROR - " FMT , ##__VA_ARGS__)
#define DBG_PRINT(LVL, FMT, ...) PRINT_SIMPLE(LVL, FMT , ##__VA_ARGS__)
// ============================================================================
#define WARN_PRINT(FMT, ...) DBG_PRINT(DBG_LEVEL_WARN, FMT , ##__VA_ARGS__)
#define INFO_PRINT(FMT, ...) DBG_PRINT(DBG_LEVEL_INFO, FMT , ##__VA_ARGS__)
#define SDBG_PRINT(FMT, ...) DBG_PRINT(DBG_LEVEL_DEBUG, FMT , ##__VA_ARGS__)
#define VDBG_PRINT(FMT, ...) DBG_PRINT(DBG_LEVEL_VDEBUG, FMT , ##__VA_ARGS__)
// ============================================================================
void dbg_print(int level, const char* fmt, ...);
void dbg_setlevel(int newLevel);
// ============================================================================

#endif
