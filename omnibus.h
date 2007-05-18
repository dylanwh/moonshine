#ifndef __SPOON_OMNIBUS_H__
#define __SPOON_OMNIBUS_H__
#include <glib.h>
#include "closure.h"

typedef struct _OmniBus OmniBus;

OmniBus *omnibus_new(void);

void omnibus_bind(OmniBus *bus, const char *name, Closure *c);
void omnibus_call(OmniBus *bus, const char *name, gpointer arg);
OmniBus *omnibus_ref(OmniBus *bus);
void omnibus_unref(OmniBus *bus);


#endif
