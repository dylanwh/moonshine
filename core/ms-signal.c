/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine/config.h"
#include "moonshine/ms-signal.h"

#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <signal.h>

static int sigin, sigout;
static GIOChannel *channel;
static GHashTable *signals;

typedef struct {
	MSSignalFunc func;
	gpointer data;
	GDestroyNotify destroy;
} MSSignalCallback;

static gboolean on_input(UNUSED GIOChannel *i, GIOCondition c, UNUSED gpointer p)/*{{{*/
{
	int sig;
	read(sigin, &sig, sizeof(sig));
	MSSignalCallback *cb = g_hash_table_lookup(signals, GINT_TO_POINTER(sig));
	if (cb)
		cb->func(sig, cb->data);
	return TRUE;
}/*}}}*/

static void on_signal(int sig)/*{{{*/
{
	write(sigout, &sig, sizeof(sig));
}/*}}}*/

void ms_signal_init(void)/*{{{*/
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
}/*}}}*/

void ms_signal_catch(int sig, MSSignalFunc func, gpointer data, GDestroyNotify destroy)/*{{{*/
{
	/* Set up the structure to specify the new action. */
	struct sigaction action;
	action.sa_handler = on_signal;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0;

	MSSignalCallback *callback = g_new(MSSignalCallback, 1);
	callback->func = func;
	callback->data = data;
	callback->destroy = destroy;
	g_hash_table_insert(signals, GINT_TO_POINTER(sig), callback);

	sigaction (sig, &action, NULL);
}/*}}}*/

void ms_signal_clear(int sig)/*{{{*/
{
	MSSignalCallback *cb = g_hash_table_lookup(signals, GINT_TO_POINTER(sig));
	struct sigaction action;
	action.sa_handler = SIG_DFL;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0;

	g_return_if_fail(cb != NULL);
	
	sigaction(sig, &action, NULL);
	g_hash_table_remove(signals, GINT_TO_POINTER(sig));

	if (cb->data && cb->destroy)
		cb->destroy(cb->data);
}/*}}}*/
