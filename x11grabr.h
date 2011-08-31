/**
 * Some portion of code was copied from libav.
 */

#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

typedef struct XGRational{
    int num; ///< numerator
    int den; ///< denominator
} XGRational;

/**
 * x11_grab stores everything
 */
typedef struct x11_grab {
    int frame_size;          /**< Size in bytes of a grabbed frame */
    XGRational time_base;    /**< Time base */
    int64_t time_frame;      /**< Current time */

    char *video_size;        /**< String describing video size, set by a private option. */
    int height;              /**< Height of the grab frame */
    int width;               /**< Width of the grab frame */
    int x_off;               /**< Horizontal top-left corner coordinate */
    int y_off;               /**< Vertical top-left corner coordinate */

    char *display;           /* string of Display, e.g. :0.0 */
    Display *dpy;            /**< X11 display from which x11grab grabs frames */
    XImage *image;           /**< X11 image holding the grab */
    int use_shm;             /**< !0 when using XShm extension */
    XShmSegmentInfo shminfo; /**< When using XShm, keeps track of XShm infos */
    int  draw_mouse;         /**< Set by a private option. */
    int  follow_mouse;       /**< Set by a private option. */
    int  show_region;        /**< set by a private option. */
    char *framerate;         /**< Set by a private option. */

    Window region_win;       /**< This is used by show_region option. */
} XG;


typedef struct XGStream {
    XGRational time_base;
    int pts_wrap_bits; /**< number of bits in pts (used for wrapping control) */
} XGStream;

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

#define XGMAX(a,b) ((a) > (b) ? (a) : (b))
#define XGMIN(a,b) ((a) > (b) ? (b) : (a))

/* error handling */
#if EDOM > 0
#define XGERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#else
/* Some platforms have E* and errno already negated. */
#define XGERROR(e) (e)
#endif
