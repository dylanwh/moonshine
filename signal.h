#ifndef _SPOON_SIGNAL_H
#include <signal.h>
#include <unistd.h>
#include <glib.h>

#include "closure.h"

void signal_init(void);
void signal_reset(void);
void signal_catch(int sig, Closure *c);

#endif
