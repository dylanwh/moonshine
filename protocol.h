#ifndef __SPOON_PROTOCOL_H__
#define __SPOON_PROTOCOL_H__
#include <glib.h>

const GSList *haver_parse(GString *buf) G_GNUC_PURE;
const GString *haver_format(GSList *msg) G_GNUC_PURE;
void haver_msg_free(GSList *msg);

#endif
