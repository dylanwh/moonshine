#include "moonshine.h"
#include <string.h>
#define APP "app"

/* make_keyspec turns strings like "^A" into "\001", and so on. */
static int app_make_keyspec(LuaState *L)
{
	const char *str = luaL_checkstring(L, 1);
	int len = strlen(str);
	GString *buf = g_string_sized_new(len);

	for (int i = 0; i < len; i++) {
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
}

static int app_boot(LuaState *L)
{
	g_assert(app_input);
	g_assert(app_mainloop);	
	g_assert(!running);

	running = TRUE;

	term_init();

	g_io_add_watch(app_input, G_IO_IN, on_input, L);

	moon_call(L, "on_boot", "");
	g_main_loop_run(app_mainloop);

	g_io_channel_unref(app_input);
	g_main_loop_unref(app_mainloop);
	term_reset();
	g_print("Bye!\n");	
	return 0;
}

static int app_shutdown(LuaState *L)
{
	g_assert(app_mainloop);
	moon_call(L, "on_shutdown", "");
	g_main_loop_quit(app_mainloop);
	return 0;
}

static LuaLReg functions[] = {
	{ "make_keyspec", app_make_keyspec },
	{ "boot",  app_boot },
	{ "shutdown", app_shutdown },
	{ "refresh", app_refresh },
	{ 0, 0 }
};

void applib_open(LuaState *L)
{
	luaL_register(L, APP, functions);
	lua_pop(L, 1);

	app_mainloop = g_main_loop_new(NULL, FALSE);
	app_input = g_io_channel_unix_new(fileno(stdin));
}
