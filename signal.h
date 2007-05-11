#ifndef _SPOON_SIGNAL_H
#include <signal.h>
#include <unistd.h>
#include <glib.h>
#include "closure.h"

void spoon_signal_init(void);
void spoon_signal_reset(void);
void spoon_signal_add_watch();
void spoon_signal_register(int sig, Closure *c);

#endif
