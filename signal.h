#ifndef _SPOON_SIGNAL_H
#include <signal.h>
#include <unistd.h>
#include <glib.h>
#include "moon.h"

void signal_init(Moon *m);
void signal_reset(void);
void signal_catch(int sig);

#endif
