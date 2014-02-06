#include "dbg_print.h"

#include <stdio.h>
#include <stdarg.h>

#define DEFAULT_FILE stderr

static FILE* g_dbg_print_file  = DEFAULT_FILE;
static int   g_dbg_print_level = DBG_LEVEL_VDEBUG;

void dbg_print(int level, const char* fmt, ...)
{
    va_list ap;

    if ((level != DBG_LEVEL_ERROR) 
            && (g_dbg_print_level < level))
    {
        return;
    }

    // HACK: for some reason, this variable is not init'd on Ubuntu 11
    if (g_dbg_print_file == NULL)
    {
        g_dbg_print_file = DEFAULT_FILE;
    }

    va_start(ap, fmt);
    vfprintf(g_dbg_print_file, fmt, ap);
    va_end(ap);
    fflush(g_dbg_print_file);
}

void dbg_setlevel(int newLevel)
{
    g_dbg_print_level = newLevel;
}
