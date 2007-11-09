/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "config.h"
#include "mysignal.h"
#include "moon.h"

#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <signal.h>

static int sigin, sigout;
static GIOChannel *channel;
static GHashTable *signals;

static gboolean on_input(UNUSED GIOChannel *i, GIOCondition c, UNUSED gpointer p)
{
	int sig;
	read(sigin, &sig, sizeof(sig));
	SignalCallback *cb = g_hash_table_lookup(signals, GINT_TO_POINTER(sig));
	if (cb)
		cb->func(sig, cb->data);
	return TRUE;
}

static void on_signal(int sig)
{
	write(sigout, &sig, sizeof(sig));
}

void signal_init(void)
{
	int fildes[2];
	if (pipe(fildes) != 0)
		perror("pipe");
	sigin     = fildes[0];
	sigout    = fildes[1];
	channel   = g_io_channel_unix_new(sigin);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);
	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);
	signals   = g_hash_table_new(NULL, NULL);
	g_io_add_watch(channel, G_IO_IN, on_input, NULL);
}
/*
void signal_reset(void)
{
	g_io_channel_unref(channel);
	close(sigin);
	close(sigout);
}
*/

void signal_catch(int signum, SignalFunc func, gpointer data)
{
	/* Set up the structure to specify the new action. */
	struct sigaction action;
	action.sa_handler = on_signal;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0;

	SignalCallback *callback = g_new(SignalCallback, 1);
	callback->func = func;
	callback->data = data;
	g_hash_table_insert(signals, GINT_TO_POINTER(signum), callback);

	sigaction (signum, &action, NULL);
}

