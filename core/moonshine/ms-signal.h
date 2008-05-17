#ifndef __MOONSHINE_SIGNAL_H__
#define __MOONSHINE_SIGNAL_H__
#include <glib.h>
#include <signal.h>

typedef void (*MSSignalFunc)(int sig, gpointer data);
void ms_signal_init(void);
void ms_signal_catch (int sig, MSSignalFunc func, gpointer data);

#endif
