/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moon.h"
#include "term.h"
#include "config.h"
#include <glib.h>

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
}
/* }}} */
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
static int refresh(LuaState *L)/*{{{*/
{
	term_refresh();
	return 0;
}/*}}}*/
static int quit(LuaState *L)/*{{{*/
{
	g_main_loop_quit(loop);
	return 0;
}/*}}}*/

int main(int argc, char *argv[])
{
	g_thread_init(NULL);

	GError *error     = NULL;
	GIOChannel *input = g_io_channel_unix_new(fileno(stdin));
	LuaState *L       = moon_new();

	GOptionContext *context = g_option_context_new ("- a console haver client");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free(context);

	
	loop = g_main_loop_new(NULL, FALSE);

	lua_register(L, "quit", quit);
	lua_register(L, "refresh", refresh);
	lua_register(L, "make_keyspec", make_keyspec);
	lua_pushstring(L, VERSION);
	lua_setglobal(L, "VERSION");
	moon_require(L, "moonshine");

	term_init();

	g_io_add_watch(input, G_IO_IN, on_input, L);

	moon_call(L, "boot_hook", "");
	g_main_loop_run(loop);
	moon_call(L, "quit_hook", "");

	g_io_channel_unref(input);
	g_main_loop_unref(loop);
	lua_close(L);


	exit(0);
}

