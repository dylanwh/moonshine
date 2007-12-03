/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moon.h"
#include "term.h"
#include "mysignal.h"
#include "config.h"
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>

/* Option definitions. {{{1 */
static char *hostname = "chat.haverdev.org";
static int port = 7575;

static GOptionEntry entries[] = 
{
	{ "hostname", 'H', 0, G_OPTION_ARG_STRING, &hostname, "hostname to use ", "host" },
	{ "port", 'p', 0, G_OPTION_ARG_INT, &port, "connect to port P", "P" },
	{ NULL }
};
/* }}} */

static GMainLoop *loop = NULL;
static void on_log(const gchar *domain, GLogLevelFlags level, const gchar *message, gpointer data)/*{{{*/
{
	static gboolean ok = TRUE;

	if (ok) {
		LuaState *L = data;
		lua_getglobal(L, "log_hook");
		lua_pushstring(L, domain);
		lua_pushnil(L);
		lua_pushstring(L, message);
		if (lua_pcall(L, 3, 0, 0) != 0) {
			ok = FALSE;
			g_error("error running log_hook: %s", lua_tostring(L, -1));
		}
	} else {
		g_log_default_handler(domain, level, message, NULL);
	}
}/*}}}*/
static gboolean on_input(UNUSED GIOChannel *src, GIOCondition cond, gpointer data) /*{{{*/
{
	LuaState *L = data;
	if (cond & G_IO_IN) {
		do {
			gunichar c = term_getkey();
			char buf[8];
			for (int i = 0; i < sizeof(buf); i++)
				buf[i] = 0;
			g_unichar_to_utf8(c, buf);
			moon_call(L, "input_hook", "s", buf);
		} while (term_input_pending(1));
		return TRUE;
	}
	return FALSE;
}/* }}} */
static void on_resize(int sig, gpointer data)/*{{{*/
{
	LuaState *L = data;
	term_resize();
	moon_call(L, "resize_hook", "");
}/*}}}*/

/* make_keyspec turns strings like "^A" into "\001", and so on. */
static int make_keyspec(LuaState *L)/* {{{ */
{
	const char *str = luaL_checkstring(L, 1);
	gsize len       = lua_objlen(L, 1); 
	GString *buf    = g_string_sized_new(len);

	for (gsize i = 0; i < len; i++) {
		if (str[i] == '^') {
			g_string_append_c(buf, str[i + 1] ^ 64);
			i++;
		} else {
			g_string_append_c(buf, str[i]);
		}
	}
	lua_pushstring(L, buf->str);
	g_string_free(buf, TRUE);
	return 1;
}/*}}}*/
static int quit(LuaState *L)/*{{{*/
{
	g_main_loop_quit(loop);
	return 0;
}/*}}}*/
static int shell_parse(LuaState *L)/*{{{*/
{
	const char *line = luaL_checkstring(L, 1);
	int argc;
	char **argv;
	if (g_shell_parse_argv (line, &argc, &argv, NULL)) {
		lua_createtable(L, argc, 0);
		for (int i = 0; i < argc; i++) {
			lua_pushstring(L, argv[i]);
			lua_rawseti(L, -2, i+1);
		}
		g_strfreev(argv);
		return 1;
	} else {
		return 0;
	}
}/*}}}*/

int main(int argc, char *argv[])
{
	g_thread_init(NULL);
	signal_init();
	term_init_colors();
	if (!getenv("LUA_PATH"))
		setenv("LUA_PATH", "lua/?.lua;../lua/?.lua", 1);

	GError *error     = NULL;
	GIOChannel *input = g_io_channel_unix_new(fileno(stdin));
	LuaState *L       = moon_new();

	GOptionContext *context = g_option_context_new ("- a console haver client");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free(context);

	loop = g_main_loop_new(NULL, FALSE);

	lua_register(L, "quit", quit);
	lua_register(L, "make_keyspec", make_keyspec);
	lua_register(L, "shell_parse", shell_parse);
	lua_pushstring(L, VERSION);
	lua_setglobal(L, "VERSION");
	if (moon_require(L, "moonshine")) {
		term_init();
		
		signal_catch(SIGWINCH, on_resize, L);
		g_log_set_default_handler(on_log, L);

		g_io_add_watch_full(input, G_PRIORITY_HIGH, G_IO_IN, on_input, L, NULL);

		if (moon_call(L, "boot_hook", "")) {
			g_main_loop_run(loop);
			moon_call(L, "quit_hook", "");
		}

		g_io_channel_unref(input);
		g_main_loop_unref(loop);
		lua_close(L);

		exit(0);
	} else {
		exit(1);
	}
}
