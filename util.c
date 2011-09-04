#include "util.h"
#include <stdarg.h>
#include <stdio.h>

/**
 * Logging function
 */
/* TODO use #define to extend for __FILE__, __LINE__ */
void
xg_log(int level, const char *fmt, ...)
{
    va_list vl;

    if(level>xg_log_level)
        return;
    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);
}


