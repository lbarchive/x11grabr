/**
 * Some portion of code was copied from libav.
 */

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
