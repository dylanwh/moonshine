#include <glib.h>
#include <stdio.h>
#include <slang.h>
#include "config.h"
#include "signal.h"
#include "moon.h"

static int sigin, sigout;
static GIOChannel *channel;
static Moon *moon;

static gboolean on_input(GIOChannel *i, GIOCondition c, gpointer p);
static void on_signal(int sig);

void signal_init(Moon *m)
{
	int fildes[2];
	if (pipe(fildes) != 0)
		perror("pipe");
	sigin   = fildes[0];
	sigout  = fildes[1];
	channel = g_io_channel_unix_new(sigin);
	moon    = m;
	g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_input, NULL);
}

void signal_reset(void)
{
	g_io_channel_unref(channel);
	close(sigin);
	close(sigout);
}

void signal_catch(int sig)
{
	SLsignal(sig, on_signal);
}

static gboolean on_input(UNUSED GIOChannel *i, GIOCondition c, UNUSED gpointer p)
{
	int sig;
	switch (c) {
		case G_IO_IN:
			read(sigin, &sig, sizeof(sig));
			char *name = g_strconcat("signal ", g_strsignal(sig), NULL);
			moon_call(moon, name, GINT_TO_POINTER(sig));
			g_free(name);
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
