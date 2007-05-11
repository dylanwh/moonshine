#include <glib.h>
#include <stdio.h>
#include <slang.h>
#include "config.h"
#include "signal.h"

static GHashTable *signals;
static int sigin, sigout;
static GIOChannel *channel;

void spoon_signal_init(void)
{
	int fildes[2];
	signals = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) spoon_closure_unref);
	if (pipe(fildes) != 0) {
		perror("pipe");
	}
	sigin   = fildes[0];
	sigout  = fildes[1];
	channel = g_io_channel_unix_new(sigin);
}

void spoon_signal_reset(void)
{
	g_io_channel_unref(channel);
	g_hash_table_destroy(signals);
	close(sigin);
	close(sigout);
}

static gboolean on_input(UNUSED GIOChannel *i, GIOCondition c, UNUSED gpointer p)
{
	int sig;
	switch (c) {
		case G_IO_IN:
			read(sigin, &sig, sizeof(sig));
			Closure *c = (Closure *)g_hash_table_lookup(signals, GINT_TO_POINTER(sig));
			g_assert(c != NULL);
			spoon_closure_call(c, GINT_TO_POINTER(sig));
			return TRUE;
		default:
			return FALSE;
	}
}

static void on_signal(int sig)
{
	write(sigout, &sig, sizeof(sig));
}

void spoon_signal_add_watch()
{
	g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_input, NULL);
}

void spoon_signal_register(int sig, Closure *c)
{
	Closure *old = (Closure *)g_hash_table_lookup(signals, GINT_TO_POINTER(sig));

	if (old) 
		spoon_closure_unref(old);
	spoon_closure_ref(c);

	g_hash_table_insert(signals, GINT_TO_POINTER(sig), (gpointer) c);
	SLsignal(sig, on_signal);
}


