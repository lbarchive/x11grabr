/**
 * Some portion of code was copied from libav.
 */

#include "util.h"
#include <argp.h>
#include <stdbool.h>
#include <libavutil/rational.h>

static struct argp_option options[] = {
    { "display",    'i',  "DISPLAY",  0,                    "Display" },
    { "size",       's',  "SIZE",     0,                    "Video size" },
    { 0,            'x',  "X",        0,                    "X" },
    { 0,            'y',  "Y",        0,                    "Y" },
    { "framerate",  'r',  "FPS",      0,                    "Frame per second" },
    { "nomouse",    'M',  0,          OPTION_ARG_OPTIONAL,  "Do not draw mouse pointer" },
    { "nomousehighlight",
                    'm',  0,          OPTION_ARG_OPTIONAL,  "Do not draw mouse pointer highlight" },
    { "follow",     'f',  "center|PIXELS",
                                      OPTION_ARG_OPTIONAL,  "\nFollow mouse mode" },
    { "nofollow",   'F',  0,          OPTION_ARG_OPTIONAL,  "Turn off follow mouse" },
    { "border",     'b',  "STYLE",    OPTION_ARG_OPTIONAL,  "Border style" },

    { "benchmark",  256,  0,          OPTION_ARG_OPTIONAL,  "Benchmarking" },
    { 0 }
};

enum XG_BORDER_STYLE {
    XG_BORDER_NONE,
    XG_BORDER_DASHED,
    XG_BORDER_DEFAULT = XG_BORDER_DASHED
};

typedef struct {
    const char *name;
    int id;
} XGBorderStyle;

static const XGBorderStyle XG_BORDER_STYLES[] = {
    { "none",     XG_BORDER_NONE },
    { "dashed",   XG_BORDER_DASHED },
    { "default",  XG_BORDER_DEFAULT }
};

struct arguments {
    char *display;
    /* video size type string */
    char *video_size;
    int   width, height;
    int   x, y;
    /* video frame rate string */
    char *frame_rate;
    AVRational framerate;
    bool  draw_mouse;
    bool  draw_mouse_highlight;
    /**
     * == false
     *  0 - Do not follow
     * == true
     * -1 - Follow as mouse moves
     *  n - Follow when reaches within n pixels to edge
     */
    #define XDG_FOLLOW_MOUSE_CENTER -1
    int   follow_mouse;

    enum XG_BORDER_STYLE border_style;

    bool benchmark;
};

error_t parse_opt(int key, char *arg, struct argp_state *state);
void default_arguments(struct arguments *arguments);

static struct argp argp = { options, parse_opt, 0, 0 };
