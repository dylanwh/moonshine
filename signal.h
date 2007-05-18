#ifndef _SPOON_SIGNAL_H
#include <signal.h>
#include <unistd.h>
#include <glib.h>
#include "omnibus.h"

void signal_init(OmniBus *bus);
void signal_reset(void);

#define signal_catch(sig) signal_catch_full(sig, #sig)
void signal_catch_full(int sig, const char *name);

#endif
