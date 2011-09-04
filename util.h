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

/**
 * rational number numerator/denominator
 */
typedef struct XGRational{
    long num; ///< numerator
    long den; ///< denominator
} XGRational;

static inline double
xg_q2d(XGRational xg_q)
{
    return xg_q.num / (double) xg_q.den;
}

#define XG_ARRAY_ELEMS(a) (sizeof a / sizeof a[0])

#define XGMAX(a,b) ((a) > (b) ? (a) : (b))
#define XGMIN(a,b) ((a) > (b) ? (b) : (a))

/* error handling */
#if EDOM > 0
#define XGERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#else
/* Some platforms have E* and errno already negated. */
#define XGERROR(e) (e)
#endif
#endif
