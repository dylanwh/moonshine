#ifndef _HAVER_PROTOCOL_H
#define _HAVER_PROTOCOL_H

#include <glib.h>
GSList *haver_parse(char *str);
GString *haver_format(GSList *msg);


#endif
