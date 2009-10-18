#include "moonshine/lua.h"
#include "moonshine/signal.h"
#include "moonshine/term.h"
#include "moonshine/config.h"
#include "moonshine/log.h"

#include "preload.h"

#include <stdio.h>
#include <stdlib.h>

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
        return TRUE;
    }
    return FALSE;
}/* }}} */

int main(UNUSED int argc, UNUSED char *argv[])
{
    LuaState *L     = ms_lua_newstate();
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    MSLog *log      = ms_log_new();
    GIOChannel *input;
    guint tag;

    ms_signal_init(); // initialize moonshine signal callback
    ms_term_init();   // initialize the display.

    g_log_set_default_handler(ms_log_handler, (gpointer)log);

    MS_PRELOAD_ALL(L); // preload moonshine modules.

    /* stash loop and log in-lieu of globals. */
    ms_lua_stash_set(L, "loop", loop);
    ms_lua_stash_set(L, "log",  log);

    /* Store moonshine version in VERSION lua global. */
    lua_pushstring(L, MOONSHINE_VERSION);
    lua_setglobal(L, "VERSION");

    /* watch stdin */
    input = g_io_channel_unix_new(fileno(stdin));
    tag   = g_io_add_watch_full(input, G_PRIORITY_DEFAULT, G_IO_IN, on_input, (gpointer) L, NULL);

    /* catch the interesting signals */
    ms_signal_catch(SIGWINCH, on_resize,  (gpointer) L, NULL);
    ms_signal_catch(SIGTERM,  on_stopsig, (gpointer) L, NULL);
    ms_signal_catch(SIGINT,   on_stopsig, (gpointer) L, NULL);
    ms_signal_catch(SIGHUP,   on_stopsig, (gpointer) L, NULL);

    /* Start the show */
    lua_getglobal(L,  "require");
    lua_pushstring(L, "moonshine");
    if(lua_pcall(L, 1, 0, 0) != 0)
        g_critical("lua error in require 'moonshine': %s", lua_tostring(L, -1));

    /* Cleanup time */
    g_io_channel_unref(input); // free memory.
    ms_term_reset();           // reset the terminal to a sane state.
    ms_signal_reset();         // remove signal handlers and memory used.
    ms_log_free(log);          // free memory and replay any entries in the log.
    g_main_loop_unref(loop);   // free memory

    exit(0);
}
