/**
 * Some portion of code was copied from libav.
 */

#ifndef XG_UTIL_H
#define XG_UTIL_H

/**
 * Logging
 */
#define XG_LOG_FATAL     8
#define XG_LOG_ERROR    16
#define XG_LOG_WARNING  24
#define XG_LOG_INFO     32
#define XG_LOG_VERBOSE  40
#define XG_LOG_DEBUG    48
static int xg_log_level = XG_LOG_INFO;

void xg_log(int level, const char *fmt, ...);
#endif
