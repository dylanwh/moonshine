/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine.h"

/* {{{ on_input */
static gboolean on_input(UNUSED GIOChannel *src, GIOCondition cond, gpointer data)
{
	LuaState *L = data;
	if (cond & G_IO_IN) {
		do {
			gunichar c = term_getkey();
			char buf[8];
			for (int i = 0; i < sizeof(buf); i++)
				buf[i] = 0;
			g_unichar_to_utf8(c, buf);
			moon_call(L, "on_input", "s", buf);
		} while (term_input_pending(1));
		//moon_call(L, "on_input_reset", "");
		return TRUE;
	}
	return FALSE;
}
/* }}} */

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

int main(int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context = g_option_context_new ("- a console haver client");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free(context);

	GMainLoop *loop   = g_main_loop_new(NULL, FALSE);
	GIOChannel *input = g_io_channel_unix_new(fileno(stdin));
	LuaState *L       = moon_new();

	moon_require(L, "moonshine");

	term_init();
	atexit(term_reset);

	g_io_add_watch(input, G_IO_IN, on_input, L);
	moon_call(L, "on_boot", "");

	g_main_loop_run(loop);

	moon_call(L, "on_shutdown", "");
	g_io_channel_unref(input);
	g_main_loop_unref(loop);
	lua_close(L);
	return 0;
}

