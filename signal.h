#ifndef _SPOON_SIGNAL_H
#include <signal.h>
#include <unistd.h>
#include <glib.h>

void spoon_signal_init(void);
void spoon_signal_reset(void);
void spoon_signal_add_watch(void);
void spoon_signal_register(int sig, SLSig_Fun_Type *handler);

#endif
