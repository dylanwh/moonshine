#ifndef __MOONSHINE_SIGNAL_H__
#define __MOONSHINE_SIGNAL_H__
#include <glib.h>
#include <signal.h>

typedef void (*SignalFunc)(int sig, gpointer data);
typedef struct {
	SignalFunc func;
	gpointer data;
} SignalCallback;

void signal_init(void);
void signal_catch (int sig, SignalFunc func, gpointer data);

#endif
