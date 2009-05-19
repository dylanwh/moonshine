#include "moonshine/lua.h"
#include "moonshine/signal.h"
#include "moonshine/term.h"
#include "moonshine/config.h"
#include "moonshine/log.h"

#include <stdio.h>
#include <stdlib.h>

/* global: defined here in main.c, used in modloop.c. */
GMainLoop *ms_main_loop = NULL;

/* {{{ preloaded modules */
int luaopen_moonshine_idle(LuaState *);
int luaopen_moonshine_loop(LuaState *);
int luaopen_moonshine_net_client(LuaState *);
int luaopen_moonshine_parseopt_core(LuaState *);
int luaopen_moonshine_timer(LuaState *);
int luaopen_moonshine_tree(LuaState *);
int luaopen_moonshine_ui_buffer(LuaState *);
int luaopen_moonshine_ui_entry(LuaState *);
int luaopen_moonshine_ui_statusbar(LuaState *);
int luaopen_moonshine_ui_term(LuaState *);
/* }}} */

static void on_resize(UNUSED int sig, gpointer ud)/*{{{*/
{
	LuaState *L = ud;
	lua_getglobal(L, "on_resize");
	if (!lua_isnil(L, -1)) {
		if (lua_pcall(L, 0, 0, 0) != 0)
			g_warning("lua error in on_resize(): %s", lua_tostring(L, -1));
	}
	else {
		lua_pop(L, 1);
	}
}/*}}}*/

static void on_shutdown(int sig, gpointer ud)
{
	LuaState *L = ud;
	lua_getglobal(L, "on_shutdown");
	if (!lua_isnil(L, -1)) {
		if (lua_pcall(L, 0, 0, 0) != 0)
			g_warning("lua error in on_shutdown(), signal %d: %s", sig, lua_tostring(L, -1));
	}
	else {
		lua_pop(L, 1);
	}

	g_main_loop_quit(ms_main_loop);
}

static gboolean on_input(UNUSED GIOChannel *src, GIOCondition cond, gpointer ud) /*{{{*/
{
	LuaState *L = ud;

	if (cond & G_IO_IN) {
		do {
			gunichar c = ms_term_getkey();
			char buf[8];
			memset(buf, 0, sizeof(buf));
			g_unichar_to_utf8(c, buf);

			lua_getglobal(L, "on_input");
			if (!lua_isnil(L, -1)) {
				lua_pushstring(L, buf);
				if (lua_pcall(L, 1, 0, 0) != 0) {
					g_warning("lua error in on_input(): %s", lua_tostring(L, -1));
					//return TRUE;
				}
			}
			else {
				lua_pop(L, 1);
			}
		} while (ms_term_input_pending(-1));
		//moon_call(L, "input_reset_hook", "");
		return TRUE;
	}
	return FALSE;
}/* }}} */


int main(int argc, char *argv[])
{
	LuaState *L     = ms_lua_newstate();

	ms_lua_preload(L, "moonshine.idle",         luaopen_moonshine_idle);
	ms_lua_preload(L, "moonshine.loop",         luaopen_moonshine_loop);
	ms_lua_preload(L, "moonshine.net.client",   luaopen_moonshine_net_client);
	ms_lua_preload(L, "moonshine.parseopt.core",luaopen_moonshine_parseopt_core);
	ms_lua_preload(L, "moonshine.timer",        luaopen_moonshine_timer);
	ms_lua_preload(L, "moonshine.tree",         luaopen_moonshine_tree);
	ms_lua_preload(L, "moonshine.ui.buffer",    luaopen_moonshine_ui_buffer);
	ms_lua_preload(L, "moonshine.ui.entry",     luaopen_moonshine_ui_entry);
	ms_lua_preload(L, "moonshine.ui.statusbar", luaopen_moonshine_ui_statusbar);
	ms_lua_preload(L, "moonshine.ui.term",      luaopen_moonshine_ui_term);

	g_thread_init(NULL);
	ms_main_loop = g_main_loop_new(NULL, FALSE);
	ms_signal_init();

	GIOChannel *input = g_io_channel_unix_new(fileno(stdin));
	g_io_add_watch_full(input, G_PRIORITY_DEFAULT, G_IO_IN, on_input,
			(gpointer) L, NULL);
	//g_io_channel_unref(input); // XXX: I assume g_io_add_watch_full() refs input.

	ms_signal_catch(SIGWINCH, on_resize, (gpointer) L, NULL);
	ms_signal_catch(SIGTERM, on_shutdown, (gpointer) L, NULL);
	ms_signal_catch(SIGINT, on_shutdown, (gpointer) L, NULL);
	ms_signal_catch(SIGHUP, on_shutdown, (gpointer) L, NULL);

	MSLog *log = ms_log_new();
	g_log_set_default_handler(ms_log_handler, (gpointer)log);
	ms_term_init();

	/* Start the show */
	lua_getglobal(L, "require");
	lua_pushstring(L, "moonshine");
	if(lua_pcall(L, 1, 0, 0) != 0)
		g_warning("lua error in require 'moonshine': %s", lua_tostring(L, -1));

	/* Cleanup time */
	ms_term_reset(); 
	ms_signal_reset();

	ms_log_free(log);
	g_main_loop_unref(ms_main_loop);

	exit(0);
}
