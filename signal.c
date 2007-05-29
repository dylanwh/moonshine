/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include <stdio.h>

#include <glib.h>
#include <slang.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "config.h"
#include "signal.h"
#include "moon.h"

static GHashTable *names;
static int sigin, sigout;
static GIOChannel *channel;
static lua_State *lua_state;

static int on_signal(lua_State *L)
{
	const char *signal = luaL_checkstring(L, 1);
	char *name = g_strconcat("on_signal_", signal, NULL);
	if (!moon_call(L, name, ""))
		moon_call(L, "quit", "");
	return 0;
}

static gboolean got_input(UNUSED GIOChannel *i, GIOCondition c, UNUSED gpointer p)
{
	int sig;
	switch (c) {
		case G_IO_IN:
			read(sigin, &sig, sizeof(sig));
			char *signame = g_hash_table_lookup(names, GINT_TO_POINTER(sig));
			g_assert(signame != NULL);
			moon_call(lua_state, "on_signal", "s", signame);
			return TRUE;
		default:
			return FALSE;
	}
}

static void got_signal(int sig)
{
	write(sigout, &sig, sizeof(sig));
	SLsignal(sig, got_signal);
}

void signal_init(lua_State *L)
{
	g_assert(names == NULL);
	int fildes[2];
	if (pipe(fildes) != 0)
		perror("pipe");
	sigin   = fildes[0];
	sigout  = fildes[1];
	channel = g_io_channel_unix_new(sigin);
	names   = g_hash_table_new(g_direct_hash, g_direct_equal);
	lua_state = L;
	lua_register(L, "on_signal", on_signal);
	g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, got_input, NULL);
}

void signal_reset(void)
{
	g_assert(names != NULL);
	g_hash_table_destroy(names);
	g_io_channel_unref(channel);
	close(sigin);
	close(sigout);
}

void signal_catch_full(int sig, const char *name)
{
	g_assert(names != NULL);
	g_assert(name  != NULL);
	g_hash_table_insert(names, GINT_TO_POINTER(sig), (gpointer) name);
	SLsignal(sig, got_signal);
}


