/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moon.h"
#include "term.h"
#include "mysignal.h"
#include "config.h"
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>

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
		} while (term_input_pending(-1));
		moon_call(L, "input_reset_hook", "");
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

/* We use $MOONSHINE_RUNTIME as lua's package.path.
 * If it isn't set, we default to $HOME/.moonshine/ and the compile-time
 * constant MOONSHINE_RUNTIME. */
static char *package_path(void)
{
	const char *path = g_getenv("MOONSHINE_RUNTIME");
	if (path) {
		return g_strdup(path);
	} else {
		const char *home = g_getenv("HOME");
		if (home == NULL) {
			home = g_get_home_dir();
			g_assert(home != NULL);
		}
		return g_strjoin("", home, "/.moonshine/?.lua;" MOONSHINE_RUNTIME, NULL);
	}
}

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

	char *path = package_path();
	LuaState *L = moon_new(path);
	g_free(path);

	lua_register(L, "quit", quit);
	lua_register(L, "make_keyspec", make_keyspec);
	lua_register(L, "shell_parse", shell_parse);

	GIOChannel *input = g_io_channel_unix_new(fileno(stdin));
	loop = g_main_loop_new(NULL, FALSE);

	if (moon_require(L, "boot")) {
		term_init();
		
		signal_catch(SIGWINCH, on_resize, L);
		g_log_set_default_handler(on_log, L);

		g_io_add_watch_full(input, G_PRIORITY_DEFAULT, G_IO_IN, on_input, L, NULL);

		if (moon_call(L, "init_hook", "")) {
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
