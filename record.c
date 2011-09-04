#include "x11grabr.h"
#include "record.h"
#include <stdlib.h>

typedef struct {
    Display           *dpy_data;
    Display           *dpy_ctrl;
    XRecordRange      *rr;
    XRecordClientSpec  rcs;
    XRecordContext     rc;
} XGRecord;

XGRecord xgr;

bool
xg_record_init(XG *xg)
{
    int major, minor;

    xgr.dpy_ctrl = XOpenDisplay(NULL);
    xgr.dpy_data = XOpenDisplay(NULL);
    XSynchronize(xgr.dpy_ctrl, True);

    if (!XRecordQueryVersion(xgr.dpy_ctrl, &major, &minor)) {
        xg_log(XG_LOG_ERROR, "X Record Extension is not supported\n");
        goto err;
    }
   
    xg_log(XG_LOG_INFO, "X Record Extension %d.%d\n", major, minor);

    xgr.rr = XRecordAllocRange();
    if (!xgr.rr) {
        xg_log(XG_LOG_ERROR, "Could not allocate record range\n");
        goto err;
    }

    xgr.rr->device_events.first = ButtonPress;
    xgr.rr->device_events.last  = MotionNotify;
    xgr.rcs = XRecordAllClients;

    xgr.rc = XRecordCreateContext(xgr.dpy_ctrl, 0, &xgr.rcs, 1, &xgr.rr, 1);
    if (!xgr.rc) {
        xg_log(XG_LOG_ERROR, "Could not create a record context\n");
        goto err;
    }

    if (!XRecordEnableContextAsync(xgr.dpy_data, xgr.rc, xg_record_callback, (XPointer) xg)) {
        xg_log(XG_LOG_ERROR, "Cound not enable the record context\n");
        goto err1;
    }
    
    return true;

err1:
    XRecordFreeContext(xgr.dpy_ctrl, xgr.rc);
err:
    if (xgr.rr)
        XFree(xgr.rr);        
    if (xgr.dpy_ctrl)
        XCloseDisplay(xgr.dpy_ctrl);
    if (xgr.dpy_data)
        XCloseDisplay(xgr.dpy_data);
    return false;
}

void
xg_record_close(XG *xg)
{
    XRecordDisableContext(xgr.dpy_ctrl, xgr.rc);
    XRecordFreeContext(xgr.dpy_ctrl, xgr.rc);
    XFree(xgr.rr);        
    XCloseDisplay(xgr.dpy_ctrl);
    XCloseDisplay(xgr.dpy_data);
}

void
xg_record_process(void)
{
    XRecordProcessReplies(xgr.dpy_data);
}

void
xg_record_callback(XPointer private, XRecordInterceptData *hook)
{
    XG *xg = (XG *) private;

    if (hook->category != XRecordFromServer) {
        XRecordFreeData(hook);
        return;
    }

    xEvent *event = (xEvent *) hook->data;

    int event_type = event->u.u.type;
    switch (event_type) {
        case ButtonPress:
            xg->pointer_button = event->u.u.detail;
            break;
        case ButtonRelease:
            xg->pointer_button = 0;
            break;
        case MotionNotify:
            xg->pointer_x = event->u.keyButtonPointer.rootX;
            xg->pointer_y = event->u.keyButtonPointer.rootY;
            break;
        default:
            break;
    }
    XRecordFreeData(hook);
}