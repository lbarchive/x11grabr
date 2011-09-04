/*
 * X11 video GRABbeR
 * Copyright (C) 2011 Yu-Jie Lin <livibetter@gmail.com>
 *
 * Some portion of code was copied from libav.
 *
 * This file was part of Libav as libavdevice/x11grab.c
 *
 * Libav integration:
 * Copyright (C) 2006 Clemens Fruhwirth <clemens@endorphin.org>
 *                    Edouard Gomez <ed.gomez@free.fr>
 *
 * This file contains code from grab.c:
 * Copyright (c) 2000-2001 Fabrice Bellard
 *
 * This file contains code from the xvidcap project:
 * Copyright (C) 1997-1998 Rasca, Berlin
 *               2003-2004 Karl H. Beckers, Frankfurt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "x11grabr.h"
#include "record.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <sys/shm.h>
#include <X11/extensions/record.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xfixes.h>

XG xg;

int64_t xg_gettime(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

#define REGION_WIN_BORDER 3
/**
 * Draw grabbing region window
 *
 * @param s x11_grab context
 */
static void
xg_draw_region_win(XG *s)
{
    Display *dpy = s->dpy;
    int screen;
    Window win = s->region_win;
    GC gc;

    screen = DefaultScreen(dpy);
    gc = XCreateGC(dpy, win, 0, 0);
    XSetForeground(dpy, gc, WhitePixel(dpy, screen));
    XSetBackground(dpy, gc, BlackPixel(dpy, screen));
    XSetLineAttributes(dpy, gc, REGION_WIN_BORDER, LineDoubleDash, 0, 0);
    XDrawRectangle(dpy, win, gc,
                   1, 1,
                   (s->width  + REGION_WIN_BORDER * 2) - 1 * 2 - 1,
                   (s->height + REGION_WIN_BORDER * 2) - 1 * 2 - 1);
    XFreeGC(dpy, gc);
}

/**
 * Initialize grabbing region window
 *
 * @param s x11_grab context
 */
static void
xg_region_win_init(XG *s)
{
    Display *dpy = s->dpy;
    int screen;
    XSetWindowAttributes attribs;
    XRectangle rect;

    screen = DefaultScreen(dpy);
    attribs.override_redirect = True;
    s->region_win = XCreateWindow(dpy, RootWindow(dpy, screen),
                                  s->x_off  - REGION_WIN_BORDER,
                                  s->y_off  - REGION_WIN_BORDER,
                                  s->width  + REGION_WIN_BORDER * 2,
                                  s->height + REGION_WIN_BORDER * 2,
                                  0, CopyFromParent,
                                  InputOutput, CopyFromParent,
                                  CWOverrideRedirect, &attribs);
    rect.x = 0;
    rect.y = 0;
    rect.width  = s->width;
    rect.height = s->height;
    XShapeCombineRectangles(dpy, s->region_win,
                            ShapeBounding, REGION_WIN_BORDER, REGION_WIN_BORDER,
                            &rect, 1, ShapeSubtract, 0);
    XMapWindow(dpy, s->region_win);
    XSelectInput(dpy, s->region_win, ExposureMask | StructureNotifyMask);
    xg_draw_region_win(s);
    xg_log(XG_LOG_DEBUG, "Init region win at (%d, %d), size = %dx%d\n", s->x_off, s->y_off, s->width, s->height);
}

/**
 * Initialize the x11 grab device demuxer (public device demuxer API).
 *
 * @param s1 Context from avformat core
 * @param ap Parameters from avformat core
 * @return <ul>
 *          <li>XGERROR(ENOMEM) no memory left</li>
 *          <li>XGERROR(EIO) other failure case</li>
 *          <li>0 success</li>
 *         </ul>
 */
static int
xg_init(XG *xg)
{
    Display *dpy;
    XGStream *st = NULL;
    XImage *image;
    int x_off = 0;
    int y_off = 0;
    int screen;
    int use_shm;
    char *param, *offset;
    int ret = 0;

    param = strdup(xg->display);

    xg_log(XG_LOG_INFO, "device: %s -> display: %s x: %d y: %d width: %d height: %d\n",
           xg->display, param, x_off, y_off, xg->width, xg->height);

    dpy = XOpenDisplay(param);
    if(!dpy) {
        xg_log(XG_LOG_ERROR, "Could not open X display.\n");
        ret = XGERROR(EIO);
        goto out;
    }

    st = malloc(sizeof (struct XGStream));
    st->time_base.num = 1;
    st->time_base.den = 1000000;
    st->pts_wrap_bits = 64;
    screen = DefaultScreen(dpy);

    if (xg->follow_mouse) {
        int screen_w, screen_h;
        Window w;

        screen_w = DisplayWidth(dpy, screen);
        screen_h = DisplayHeight(dpy, screen);
        XQueryPointer(dpy, RootWindow(dpy, screen), &w, &w, &x_off, &y_off, &ret, &ret, &ret);
        x_off -= xg->width / 2;
        y_off -= xg->height / 2;
        x_off = XGMIN(XGMAX(x_off, 0), screen_w - xg->width);
        y_off = XGMIN(XGMAX(y_off, 0), screen_h - xg->height);
        xg_log(XG_LOG_INFO, "followmouse is enabled, resetting grabbing region to x: %d y: %d\n", x_off, y_off);
    }

    use_shm = XShmQueryExtension(dpy);
    xg_log(XG_LOG_INFO, "shared memory extension %s found\n", use_shm ? "" : "not");

    if(use_shm) {
        int scr = XDefaultScreen(dpy);
        image = XShmCreateImage(dpy,
                                DefaultVisual(dpy, scr),
                                DefaultDepth(dpy, scr),
                                ZPixmap,
                                NULL,
                                &xg->shminfo,
                                xg->width, xg->height);
        xg->shminfo.shmid = shmget(IPC_PRIVATE,
                                        image->bytes_per_line * image->height,
                                        IPC_CREAT|0777);
        if (xg->shminfo.shmid == -1) {
            xg_log(XG_LOG_ERROR, "Fatal: Can't get shared memory!\n");
            ret = XGERROR(ENOMEM);
            goto out;
        }
        xg->shminfo.shmaddr = image->data = shmat(xg->shminfo.shmid, 0, 0);
        xg->shminfo.readOnly = False;

        if (!XShmAttach(dpy, &xg->shminfo)) {
            xg_log(XG_LOG_ERROR, "Fatal: Failed to attach shared memory!\n");
            /* needs some better error subroutine :) */
            ret = XGERROR(EIO);
            goto out;
        }
    } else {
        image = XGetImage(dpy, RootWindow(dpy, screen),
                          x_off,y_off,
                          xg->width, xg->height,
                          AllPlanes, ZPixmap);
    }

    xg->frame_size = xg->width * xg->height * image->bits_per_pixel/8;
    xg->dpy = dpy;
    xg->time_frame = xg_gettime() / xg_q2d(xg->time_base);
    xg->x_off = x_off;
    xg->y_off = y_off;
    xg->image = image;
    xg->image_s = cairo_image_surface_create_for_data(
                      image->data,
                      CAIRO_FORMAT_ARGB32,
                      xg->width,
                      xg->height,
                      cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
                                                    xg->width));
    xg->cr = cairo_create(xg->image_s);
    xg->use_shm = use_shm;

out:
    return ret;
}

/**
 * Paint a mouse pointer in an X11 image.
 *
 * @param image image to paint the mouse pointer to
 * @param s context used to retrieve original grabbing rectangle
 *          coordinates
 */
static void
paint_mouse_pointer(XImage *image, XG *s)
{
    int x_off = s->x_off;
    int y_off = s->y_off;
    int width = s->width;
    int height = s->height;
    Display *dpy = s->dpy;
    XFixesCursorImage *xcim;
    int x, y;
    int line, column;
    int to_line, to_column;
    int pixstride = image->bits_per_pixel >> 3;
    /* Warning: in its insanity, xlib provides unsigned image data through a
     * char* pointer, so we have to make it uint8_t to make things not break.
     * Anyone who performs further investigation of the xlib API likely risks
     * permanent brain damage. */
    uint8_t *pix = image->data;

    /* Code doesn't currently support 16-bit or PAL8 */
    if (image->bits_per_pixel != 24 && image->bits_per_pixel != 32)
        return;

    xcim = XFixesGetCursorImage(dpy);

    x = xcim->x - xcim->xhot;
    y = xcim->y - xcim->yhot;

    to_line = XGMIN((y + xcim->height), (height + y_off));
    to_column = XGMIN((x + xcim->width), (width + x_off));

    for (line = XGMAX(y, y_off); line < to_line; line++) {
        for (column = XGMAX(x, x_off); column < to_column; column++) {
            int  xcim_addr = (line - y) * xcim->width + column - x;
            int image_addr = ((line - y_off) * width + column - x_off) * pixstride;
            int r = (uint8_t)(xcim->pixels[xcim_addr] >>  0);
            int g = (uint8_t)(xcim->pixels[xcim_addr] >>  8);
            int b = (uint8_t)(xcim->pixels[xcim_addr] >> 16);
            int a = (uint8_t)(xcim->pixels[xcim_addr] >> 24);

            if (a == 255) {
                pix[image_addr+0] = r;
                pix[image_addr+1] = g;
                pix[image_addr+2] = b;
            } else if (a) {
                /* pixel values from XFixesGetCursorImage come premultiplied by alpha */
                pix[image_addr+0] = r + (pix[image_addr+0]*(255-a) + 255/2) / 255;
                pix[image_addr+1] = g + (pix[image_addr+1]*(255-a) + 255/2) / 255;
                pix[image_addr+2] = b + (pix[image_addr+2]*(255-a) + 255/2) / 255;
            }
        }
    }

    XFree(xcim);
    xcim = NULL;
}


/**
 * Read new data in the image structure.
 *
 * @param dpy X11 display to grab from
 * @param d
 * @param image Image where the grab will be put
 * @param x Top-Left grabbing rectangle horizontal coordinate
 * @param y Top-Left grabbing rectangle vertical coordinate
 * @return 0 if error, !0 if successful
 */
static int
xget_zpixmap(Display *dpy, Drawable d, XImage *image, int x, int y)
{
    xGetImageReply rep;
    xGetImageReq *req;
    long nbytes;

    if (!image) {
        return 0;
    }

    LockDisplay(dpy);
    GetReq(GetImage, req);

    /* First set up the standard stuff in the request */
    req->drawable = d;
    req->x = x;
    req->y = y;
    req->width = image->width;
    req->height = image->height;
    req->planeMask = (unsigned int)AllPlanes;
    req->format = ZPixmap;

    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse) || !rep.length) {
        UnlockDisplay(dpy);
        SyncHandle();
        return 0;
    }

    nbytes = (long)rep.length << 2;
    _XReadPad(dpy, image->data, nbytes);

    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

/**
 * Grab a frame from x11 (public device demuxer API).
 *
 * @param s1 Context from avformat core
 * @param pkt Packet holding the brabbed frame
 * @return frame size in bytes
 */
static int
xg_read_packet(XG *s)
{
    Display *dpy = s->dpy;
    XImage *image = s->image;
    int x_off = s->x_off;
    int y_off = s->y_off;

    int screen;
    Window root;
    int follow_mouse = s->follow_mouse;

    int64_t curtime, delay;
    struct timespec ts;

    int screen_w, screen_h;
    int pointer_x = s->pointer_x;
    int pointer_y = s->pointer_y;
    int _;
    Window w;

    /* Calculate the time of the next frame */
    s->time_frame += INT64_C(1000000);

    /* wait based on the frame rate */
    for(;;) {
        curtime = xg_gettime();
        delay = s->time_frame * xg_q2d(s->time_base) - curtime;
        if (delay <= 0) {
            if (delay < INT64_C(-1000000) * xg_q2d(s->time_base)) {
                s->time_frame += INT64_C(1000000);
            }
            break;
        }
        ts.tv_sec = delay / 1000000;
        ts.tv_nsec = (delay % 1000000) * 1000;
        nanosleep(&ts, NULL);
    }

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    if (!s->record_ext)
        XQueryPointer(dpy, root, &w, &w, &pointer_x, &pointer_y, &_, &_, &_);
    if (follow_mouse) {
        screen_w = DisplayWidth(dpy, screen);
        screen_h = DisplayHeight(dpy, screen);
        if (follow_mouse == -1) {
            // follow the mouse, put it at center of grabbing region
            x_off += pointer_x - s->width  / 2 - x_off;
            y_off += pointer_y - s->height / 2 - y_off;
        } else {
            // follow the mouse, but only move the grabbing region when mouse
            // reaches within certain pixels to the edge.
            if (pointer_x > x_off + s->width - follow_mouse) {
                x_off += pointer_x - (x_off + s->width - follow_mouse);
            } else if (pointer_x < x_off + follow_mouse)
                x_off -= (x_off + follow_mouse) - pointer_x;
            if (pointer_y > y_off + s->height - follow_mouse) {
                y_off += pointer_y - (y_off + s->height - follow_mouse);
            } else if (pointer_y < y_off + follow_mouse)
                y_off -= (y_off + follow_mouse) - pointer_y;
        }
        // adjust grabbing region position if it goes out of screen.
        s->x_off = x_off = XGMIN(XGMAX(x_off, 0), screen_w - s->width);
        s->y_off = y_off = XGMIN(XGMAX(y_off, 0), screen_h - s->height);

        if (s->show_region && s->region_win) {
            XMoveWindow(dpy, s->region_win,
                        s->x_off - REGION_WIN_BORDER,
                        s->y_off - REGION_WIN_BORDER);
            xg_log(XG_LOG_DEBUG, "Move region win to %d, %d\n", s->x_off, s->y_off);
            }
    }

    if (s->show_region) {
        if (s->region_win) {
            XEvent evt;
            // clean up the events, and do the initinal draw or redraw.
            for (evt.type = NoEventMask; XCheckMaskEvent(dpy, ExposureMask | StructureNotifyMask, &evt); );
            if (evt.type)
                xg_draw_region_win(s);
        } else {
            xg_region_win_init(s);
        }
    }

    if(s->use_shm) {
        if (!XShmGetImage(dpy, root, image, x_off, y_off, AllPlanes)) {
            xg_log (XG_LOG_INFO, "XShmGetImage() failed\n");
        }
    } else {
        if (!xget_zpixmap(dpy, root, image, x_off, y_off)) {
            xg_log (XG_LOG_INFO, "XGetZPixmap() failed\n");
        }
    }

    if (true) {
        if (s->pointer_button)
            cairo_set_source_rgba (s->cr, 1, 0, 0, 0.5);
        else
            cairo_set_source_rgba (s->cr, 1, 1, 0, 0.5);
        cairo_arc (s->cr, pointer_x - s->x_off, pointer_y - s->y_off, 50, 0, 2*M_PI);
        cairo_fill (s->cr);
    }

    if (s->draw_mouse) {
        paint_mouse_pointer(image, s);
    }

    return s->frame_size;
}

/**
 * Close x11 frame grabber (public device demuxer API).
 *
 * @param s1 Context from avformat core
 * @return 0 success, !0 failure
 */
static int
xg_read_close(XG *xg)
{
    /* Detach cleanly from shared mem */
    if (xg->use_shm) {
        XShmDetach(xg->dpy, &xg->shminfo);
        shmdt(xg->shminfo.shmaddr);
        shmctl(xg->shminfo.shmid, IPC_RMID, NULL);
    }

    /* Destroy Cairo surface */
    if (xg->image_s) {
        cairo_destroy(xg->cr);
        cairo_surface_destroy(xg->image_s);
    }

    /* Destroy X11 image */
    if (xg->image) {
        XDestroyImage(xg->image);
        xg->image = NULL;
    }

    if (xg->region_win) {
        XDestroyWindow(xg->dpy, xg->region_win);
    }

    /* Free X11 display */
    XCloseDisplay(xg->dpy);
    return 0;
}

int
main(int argc, char **argv) {
    struct arguments arguments;

    default_arguments(&arguments);
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    xg_log(XG_LOG_INFO, "DISPLAY: %s\n", arguments.display);
    xg_log(XG_LOG_INFO, "VIDEO  : %s\n", arguments.video_size);
    xg_log(XG_LOG_INFO, "SIZE   : %dx%d\n",
           arguments.width, arguments.height);
    xg_log(XG_LOG_INFO, "X, Y   : %d, %d\n",
           arguments.x, arguments.y);
    xg_log(XG_LOG_INFO, "FPS    : %s (%ld / %ld)\n",
           arguments.frame_rate,
           arguments.framerate.num, arguments.framerate.den);
    xg_log(XG_LOG_INFO, "MOUSE  : %d\n", arguments.draw_mouse);
    xg_log(XG_LOG_INFO, "FOLLOW : %d\n", arguments.follow_mouse);
    xg_log(XG_LOG_INFO, "BORDER : %s (%d)\n",
           xg_border_style_name(arguments.border_style),
           arguments.border_style);

    xg.display      = strdup(arguments.display);
    xg.video_size   = strdup(arguments.video_size);
    xg.width        = arguments.width;
    xg.height       = arguments.height;
    xg.frame_rate   = arguments.frame_rate;
    xg.framerate    = arguments.framerate;
    xg.time_base    = (XGRational){xg.framerate.den, xg.framerate.num};
    xg.draw_mouse   = arguments.draw_mouse;
    xg.follow_mouse = arguments.follow_mouse;
    xg.show_region  = arguments.border_style;
    xg.region_win   = 0;

    xg_init(&xg);
    xg.record_ext = xg_record_init(&xg);

/*
    printf("%d\n", ctx.streams[0]->codec->pix_fmt);
    printf("%d\n", PIX_FMT_RGB32);
    printf("%d\n", PIX_FMT_BGRA);
    printf("%d\n", xg.image->bits_per_pixel / 8);
*/
    while (1) {
        xg_read_packet(&xg);
        if (xg.record_ext)
            xg_record_process();
//        continue;
        fwrite(xg.image->data,
               xg.image->bits_per_pixel / 8,
               xg.image->width * xg.image->height,
               stdout);
        }

    if (xg.record_ext)
        xg_record_close(&xg);

    xg_read_close(&xg);
    return 0;
}
