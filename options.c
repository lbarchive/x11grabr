/**
 * Some portion of code was copied from libav.
 */

#include "options.h"
#include <gmp.h>
#include <stdlib.h>

const char *
xg_border_style_name(int id)
{
    int i;

    for (i=0; i<XG_ARRAY_ELEMS(XG_BORDER_STYLES); i++)
        if (XG_BORDER_STYLES[i].id == id)
            return XG_BORDER_STYLES[i].name;
    return NULL;
}

error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;
    int i, n;
    XGRational *rate;
    double d;
    mpq_t q;
    mpz_t z;

    switch (key) {
        case 'i':
            arguments->display = arg;
            break;
        case 's':
            for (i=0; i<XG_ARRAY_ELEMS(XG_VIDEO_SIZES); i++)
                if (strcasecmp(XG_VIDEO_SIZES[i].name, arg) == 0) {
                    arguments->video_size = arg;
                    arguments->width      = XG_VIDEO_SIZES[i].width;
                    arguments->height     = XG_VIDEO_SIZES[i].height;
                    return 0;
                }
            return ARGP_ERR_UNKNOWN;
        case 'x':
            arguments->x = atoi(arg);
            break;
        case 'y':
            arguments->y = atoi(arg);
            break;
        case 'r':
            n = XG_ARRAY_ELEMS(XG_VIDEO_RATES);
            rate = &arguments->framerate;

            /* First, we check our abbreviation table */
            for (i = 0; i < n; ++i)
                if (!strcmp(XG_VIDEO_RATES[i].name, arg)) {
                    arguments->frame_rate = arg;
                    *rate = XG_VIDEO_RATES[i].rate;
                    return 0;
                }

            d = atof(arg);
            mpq_init(q);
            mpz_init(z);
            mpq_set_d(q, d);
            mpq_get_num(z, q);
            rate->num = mpz_get_si(z);
            mpq_get_den(z, q);
            rate->den = mpz_get_si(z);
            arguments->frame_rate = arg;
            break;
        case 'M':
            arguments->draw_mouse = false;
            break;
        case 'f':
            arguments->follow_mouse = (!arg || strcasecmp("center", arg) == 0)
                                      ? -1
                                      : atoi(arg);
            break;
        case 'c':
            arguments->follow_mouse = -1;
            break;
        case 'b':
            if (!arg) {
                arguments->border_style = XG_BORDER_DEFAULT;
                return 0;
            }
            for (i=0; i<XG_ARRAY_ELEMS(XG_BORDER_STYLES); i++)
                if (strcasecmp(XG_BORDER_STYLES[i].name, arg) == 0) {
                    arguments->border_style = XG_BORDER_STYLES[i].id;
                    return 0;
                }
            return ARGP_ERR_UNKNOWN;
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
    arguments->framerate    = (XGRational) { 25, 1 };
    arguments->draw_mouse   = true;
    arguments->follow_mouse = 100;
    arguments->border_style = XG_BORDER_DASHED;
}
