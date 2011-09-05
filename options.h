/**
 * Some portion of code was copied from libav.
 */

#include "util.h"
#include <argp.h>
#include <stdbool.h>

static struct argp_option options[] = {
    { "display",    'i',  "DISPLAY",  0,                    "Display" },
    { "size",       's',  "SIZE",     0,                    "Video size" },
    { 0,            'x',  "X",        0,                    "X" },
    { 0,            'y',  "Y",        0,                    "Y" },
    { "framerate",  'r',  "FPS",      0,                    "Frame per second" },
    { "nomouse",    'M',  0,          OPTION_ARG_OPTIONAL,  "Do not draw mouse pointer" },
    { "follow",     'f',  "center|PIXELS",
                                      OPTION_ARG_OPTIONAL,  "\nFollow mouse mode" },
    { "nofollow",   'F',  0,          OPTION_ARG_OPTIONAL,  "Turn off follow mouse" },
    { "border",     'b',  "STYLE",    OPTION_ARG_OPTIONAL,  "Border style" },

    { "benchmark",  256,  0,          OPTION_ARG_OPTIONAL,  "Benchmarking" },
    { 0 }
};

typedef struct {
    const char *name;
    int width, height;
} XGVideoSize;

static const XGVideoSize XG_VIDEO_SIZES[] = {
    { "ntsc",      720, 480 },
    { "pal",       720, 576 },
    { "qntsc",     352, 240 }, /* VCD compliant NTSC */
    { "qpal",      352, 288 }, /* VCD compliant PAL */
    { "sntsc",     640, 480 }, /* square pixel NTSC */
    { "spal",      768, 576 }, /* square pixel PAL */
    { "film",      352, 240 },
    { "ntsc-film", 352, 240 },
    { "sqcif",     128,  96 },
    { "qcif",      176, 144 },
    { "cif",       352, 288 },
    { "4cif",      704, 576 },
    { "16cif",    1408,1152 },
    { "qqvga",     160, 120 },
    { "qvga",      320, 240 },
    { "vga",       640, 480 },
    { "svga",      800, 600 },
    { "xga",      1024, 768 },
    { "uxga",     1600,1200 },
    { "qxga",     2048,1536 },
    { "sxga",     1280,1024 },
    { "qsxga",    2560,2048 },
    { "hsxga",    5120,4096 },
    { "wvga",      852, 480 },
    { "wxga",     1366, 768 },
    { "wsxga",    1600,1024 },
    { "wuxga",    1920,1200 },
    { "woxga",    2560,1600 },
    { "wqsxga",   3200,2048 },
    { "wquxga",   3840,2400 },
    { "whsxga",   6400,4096 },
    { "whuxga",   7680,4800 },
    { "cga",       320, 200 },
    { "ega",       640, 350 },
    { "hd480",     852, 480 },
    { "hd720",    1280, 720 },
    { "hd1080",   1920,1080 },
};

typedef struct {
    const char *name;
    XGRational rate;
} XGVideoRate;

static const XGVideoRate XG_VIDEO_RATES[]= {
    { "ntsc",      { 30000, 1001 } },
    { "pal",       {    25,    1 } },
    { "qntsc",     { 30000, 1001 } }, /* VCD compliant NTSC */
    { "qpal",      {    25,    1 } }, /* VCD compliant PAL */
    { "sntsc",     { 30000, 1001 } }, /* square pixel NTSC */
    { "spal",      {    25,    1 } }, /* square pixel PAL */
    { "film",      {    24,    1 } },
    { "ntsc-film", { 24000, 1001 } },
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
    XGRational framerate;
    bool  draw_mouse;
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
