/* Moonshine - a Lua-based chat client
 *
 * Copyright (C) 2010 Dylan William Hardison
 *
 * This file is part of Moonshine.
 *
 * Moonshine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonshine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "moonshine/lua.h"
#include "moonshine/signal.h"
#include "moonshine/term.h"
#include "moonshine/config.h"
#include "moonshine/log.h"

#include "preload.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

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

static void on_stopsig(UNUSED int sig, gpointer ud)/*{{{*/
{
    LuaState *L = ud;
    GMainLoop *loop = ms_lua_stash_get(L, "loop");
    g_return_if_fail(loop != NULL);
    g_main_loop_quit(loop);
}/*}}}*/

static gboolean on_input(UNUSED GIOChannel *src, GIOCondition cond, gpointer ud) /*{{{*/
{
    LuaState *L = ud;

    if (cond & G_IO_IN) {
        lua_getglobal(L, "on_input");
        if (!lua_isnil(L, -1)) {
            gunichar ch = 0;
            gboolean ok = ms_term_getkey(&ch);
            if (ok) {
                char buf[8];
                memset(buf, 0, sizeof(buf));
                g_unichar_to_utf8(ch, buf);
                lua_pushstring(L, buf);
            }
            else {
                lua_pushnil(L);
            }
            if (lua_pcall(L, 1, 0, 0) != 0) {
                g_warning("lua error in on_input(): %s", lua_tostring(L, -1));
            }
        }
        else {
            lua_pop(L, 1);
        }
        return TRUE;
    }
    return FALSE;
}/* }}} */

static int loop_quit(LuaState *L)/*{{{*/
{
    GMainLoop *loop = ms_lua_stash_get(L, "loop");
	g_return_val_if_fail(loop != NULL, 0);
    g_main_loop_quit(loop);
	return 0;
}/*}}}*/

int main(UNUSED int argc, UNUSED char *argv[])
{
    LuaState *L     = ms_lua_newstate();
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    MSLog *log      = ms_log_new();
    GIOChannel *input;
    guint tag;

	g_set_prgname("moonshine");
#if GLIB_CHECK_VERSION(2,2,0)
	g_set_application_name("Moonshine");
#endif
    setlocale(LC_ALL, "");

    MS_PRELOAD_ALL(L); // preload moonshine modules.

    /* stash loop and log in-lieu of globals. */
    ms_lua_stash_set(L, "loop", loop);
    ms_lua_stash_set(L, "log",  log);

    /* Store moonshine version in VERSION lua global. */
    lua_pushstring(L, MOONSHINE_VERSION);
    lua_setglobal(L, "VERSION");

	/* Add quit() builtin, which exists the implicit event loop */
    lua_pushcfunction(L, loop_quit);
    lua_setglobal(L, "quit");

    /* watch stdin */
    input = g_io_channel_unix_new(fileno(stdin));
    tag   = g_io_add_watch_full(input, G_PRIORITY_DEFAULT, G_IO_IN, on_input, (gpointer) L, NULL);

    /* catch the interesting signals */
    ms_signal_init(); // initialize moonshine signal callback
    ms_signal_catch(SIGWINCH, on_resize,  (gpointer)L, NULL);
    ms_signal_catch(SIGTERM,  on_stopsig, (gpointer)L, NULL);
    ms_signal_catch(SIGINT,   on_stopsig, (gpointer)L, NULL);
    ms_signal_catch(SIGHUP,   on_stopsig, (gpointer)L, NULL);

    /* Start the show */
    ms_lua_require(L, "moonshine");
    g_main_loop_run(loop);

	lua_getglobal(L, "on_quit");
    if (!lua_isnil(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != -1)
            g_warning("lua error in on_quit(): %s", lua_tostring(L, -1));
    }
    else {
        lua_pop(L, 1);
    }

    /* Cleanup time */
    g_io_channel_unref(input); // free memory.
    g_main_loop_unref(loop);   // free memory
    ms_signal_reset();         // remove signal handlers and memory used.
    lua_close(L);              // close lua.
    ms_term_reset();           // reset term here, so we can print to the screen (to replay the log)
    ms_log_free(log);          // free memory and replay any entries in the log.

    exit(0);
}
