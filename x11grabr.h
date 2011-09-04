/**
 * Some portion of code was copied from libav.
 */

#include "util.h"
#include "options.h"
#include <stdint.h>
#include <cairo.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

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
    bool record_ext;
    unsigned char pointer_button; /* it's BYTE in Xproto.h */
    int pointer_x;
    int pointer_y;
    XImage *image;           /**< X11 image holding the grab */
    cairo_surface_t *image_s;
    cairo_t *cr;
    int use_shm;             /**< !0 when using XShm extension */
    XShmSegmentInfo shminfo; /**< When using XShm, keeps track of XShm infos */
    int  draw_mouse;         /**< Set by a private option. */
    int  follow_mouse;       /**< Set by a private option. */
    int  show_region;        /**< set by a private option. */
    char *frame_rate;        /**< Set by a private option. */
    XGRational framerate;

    Window region_win;       /**< This is used by show_region option. */
} XG;


typedef struct XGStream {
    XGRational time_base;
    int pts_wrap_bits; /**< number of bits in pts (used for wrapping control) */
} XGStream;
