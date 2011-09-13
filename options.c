/**
 * Some portion of code was copied from libav.
 */

#include "options.h"
#include <stdlib.h>
#include <libavutil/common.h>
#include <libavutil/parseutils.h>

const char *
xg_border_style_name(int id)
{
    int i;

    for (i=0; i<FF_ARRAY_ELEMS(XG_BORDER_STYLES); i++)
        if (XG_BORDER_STYLES[i].id == id)
            return XG_BORDER_STYLES[i].name;
    return NULL;
}

error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;
    int i;

    switch (key) {
        case 'i':
            arguments->display = arg;
            break;
        case 's':
            if (av_parse_video_size(&arguments->width, &arguments->height, arg) < 0)
                return ARGP_ERR_UNKNOWN;
            arguments->video_size = arg;
            return 0;
        case 'x':
            arguments->x = atoi(arg);
            break;
        case 'y':
            arguments->y = atoi(arg);
            break;
        case 'r':
            if (av_parse_video_rate(&arguments->framerate, arg) < 0)
                return ARGP_ERR_UNKNOWN;
            arguments->frame_rate = arg;
            return 0;
        case 'M':
            arguments->draw_mouse = false;
            break;
        case 'f':
            arguments->follow_mouse = (!arg || strcasecmp("center", arg) == 0)
                                      ? -1
                                      : atoi(arg);
            break;
        case 'F':
            arguments->follow_mouse = 0;
            break;
        case 'c':
            arguments->follow_mouse = -1;
            break;
        case 'b':
            if (!arg) {
                arguments->border_style = XG_BORDER_DEFAULT;
                return 0;
            }
            for (i=0; i<FF_ARRAY_ELEMS(XG_BORDER_STYLES); i++)
                if (strcasecmp(XG_BORDER_STYLES[i].name, arg) == 0) {
                    arguments->border_style = XG_BORDER_STYLES[i].id;
                    return 0;
                }
            return ARGP_ERR_UNKNOWN;
        case 256:
            arguments->benchmark = true;
            return 0;
        default:
            return ARGP_ERR_UNKNOWN;
        }
    return 0;
}

void
default_arguments(struct arguments *arguments)
{
    arguments->display      = ":0.0";
    arguments->video_size   = "hd720";
    arguments->x            = 0;
    arguments->y            = 0;
    arguments->width        = 1280;
    arguments->height       = 720;
    arguments->frame_rate   = "25";
    arguments->framerate    = (AVRational) { 25, 1 };
    arguments->draw_mouse   = true;
    arguments->follow_mouse = 100;
    arguments->border_style = XG_BORDER_DASHED;

    arguments->benchmark    = false;
}
