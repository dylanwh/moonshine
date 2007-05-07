#ifndef _HAVER_PROTOCOL_H
#define _HAVER_PROTOCOL_H
#include <glib.h>

GSList *haver_parse(GString *buf);
GString *haver_format(GSList *msg);
void haver_msg_free(GSList *msg);

#endif
