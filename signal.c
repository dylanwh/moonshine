#include <glib.h>
#include <stdio.h>
#include <slang.h>
#include "config.h"
#include "signal.h"
#include "closure.h"

static GHashTable *signals;
static int sigin, sigout;
static GIOChannel *channel;

static gboolean on_input(GIOChannel *i, GIOCondition c, gpointer p);
static void on_signal(int sig);

void signal_init(void)
{
	int fildes[2];
	signals = g_hash_table_new(g_direct_hash, g_direct_equal);
	if (pipe(fildes) != 0)
		perror("pipe");
	sigin   = fildes[0];
	sigout  = fildes[1];
	channel = g_io_channel_unix_new(sigin);
	g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_input, NULL);
}

void signal_reset(void)
{
	g_io_channel_unref(channel);
	g_hash_table_destroy(signals);
	close(sigin);
	close(sigout);
}

void signal_catch(int sig, Closure *c)
{
	Closure *old;
	closure_ref(c);
	if ((old = g_hash_table_lookup(signals, GINT_TO_POINTER(sig))) != NULL) {
		g_hash_table_remove(signals, GINT_TO_POINTER(sig));
		closure_unref(old);
	}
	g_hash_table_insert(signals, GINT_TO_POINTER(sig), (gpointer) c);
	SLsignal(sig, on_signal);
}

static gboolean on_input(UNUSED GIOChannel *i, GIOCondition c, UNUSED gpointer p)
{
	int sig;
	switch (c) {
		case G_IO_IN:
			read(sigin, &sig, sizeof(sig));
			Closure *c = (Closure *) g_hash_table_lookup(signals, GINT_TO_POINTER(sig));
			g_assert(c != NULL);
			closure_call(c, GINT_TO_POINTER(sig));
			return TRUE;
		default:
			return FALSE;
	}
}

static void on_signal(int sig)
{
	write(sigout, &sig, sizeof(sig));
	SLsignal(sig, on_signal);
}
