#include <X11/Xproto.h>
#include <X11/extensions/record.h>

bool xg_record_init(XG *);
void xg_record_close(XG *);
void xg_record_process(XG *);
void xg_record_callback (XPointer, XRecordInterceptData *);
